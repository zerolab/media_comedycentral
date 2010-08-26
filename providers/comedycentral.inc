<?php
// $Id$

/**
 * @file
 *  This is Ccomedy Central provider include file for Embedded Media Video.
 */

define('EMVIDEO_COMEDYCENTRAL_MAIN_URL', 'http://www.comedycentral.com/');
define('EMVIDEO_COMEDYCENTRAL_DATA_VERSION', 1);

/**
 * hook emvideo_PROVIDER_info
 * This returns information relevant to a specific 3rd party video provider.
 *
 * @return
 *   A keyed array of strings requested by various admin and other forms.
 *    'provider' => The machine name of the provider. This must be the same as
 *      the base name of this filename, before the .inc extension.
 *    'name' => The translated name of the provider.
 *    'url' => The url to the main page for the provider.
 *    'settings_description' => A description of the provider that will be
 *      posted in the admin settings form.
 *    'supported_features' => An array of rows describing the state of certain
 *      supported features by the provider. These will be rendered in a table,
 *      with the columns being 'Feature', 'Supported', 'Notes'. In general,
 *      the 'Feature' column will give the name of the feature, 'Supported'
 *      will be Yes or No, and 'Notes' will give an optional description or
 *      caveats to the feature.
 */
function emvideo_comedycentral_info() {
  $features = array(
    array(t('Autoplay'), t('Yes'), ''),
    array(t('RSS Attachment'), t('Yes'), ''),
    array(t('Thumbnails'), t('Yes'), t('')),
    array(t('Full screen mode'), t('Yes'), t('You may customize the player to enable or disable full screen playback. Full screen mode is enabled by default.')),
  );
  return array(
    'provider' => 'comedycentral',
    'name' => t('Comedy Central'),
    'url' => EMVIDEO_COMEDYCENTRAL_MAIN_URL,
    'settings_description' => t('These settings specifically affect videos displayed from !comedycentral. You can also read more about its !api.', array('!comedycentral' => l(t('comedycentral.com'), EMVIDEO_COMEDYCENTRAL_MAIN_URL), '!api' => l(t("developer's API"), EMVIDEO_COMEDYCENTRAL_API_URL))),
    'supported_features' => $features,
  );
}

/**
 *  hook emvideo_PROVIDER_settings
 *  This should return a subform to be added to the emvideo_settings() admin
 *  settings page.
 *
 *  Note that a form field set will already be provided at $form['comedycentral'],
 *  so if you want specific provider settings within that field set, you should
 *  add the elements to that form array element.
 */
function emvideo_comedycentral_settings() {
  // We'll add a field set of player options here. You may add other options
  // to this element, or remove the field set entirely if there are no
  // user-configurable options allowed by the comedycentral provider.
  $form['comedycentral']['player_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Embedded video player options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  // This is an option to set the video to full screen. You should remove this
  // option if it is not provided by the comedycentral provider.
  $form['comedycentral']['player_options']['emvideo_comedycentral_full_screen'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow fullscreen'),
    '#default_value' => variable_get('emvideo_comedycentral_full_screen', 1),
    '#description' => t('Allow users to view video using the entire computer screen.'),
  );

  return $form;
}

/**
 *  hook emvideo_PROVIDER_extract
 *
 *  This is called to extract the video code from a pasted URL or embed code.
 *
 *  We'll be passed a URL or the embed code from a video when an editor pastes
 *  that in the field's textfield. We'll need to either pass back an array of
 *  regex expressions to match, or do the matching ourselves and return the
 *  resulting video code.
 *
 *  @param $parse
 *    An optional string with the pasted URL or embed code.
 *  @return
 *    Either an array of regex expressions to be tested, or a string with the
 *    video code to be used. If the hook tests the code itself, it should
 *    return either the string of the video code (if matched), or an empty
 *    array. Otherwise, the calling function will handle testing the embed code
 *    against each regex string in the returned array.
 */
function emvideo_comedycentral_extract($embed = '') {
  return array(
    '@media\.mtvnservices\.com/([^"|\']+)@i',
  );
}

/**
 *  hook emvideo_PROVIDER_data
 *
 *  Provides an array to be serialised and made available with $item elsewhere.
 *
 *  This data can be used to store any extraneous information available
 *  specifically to the comedycentral provider.
 */
function emvideo_comedycentral_data($field, $item) {
  // Initialize the data array.
  $data = array();

  // Create some version control. Thus if we make changes to the data array
  // down the road, we can respect older content. If allowed by Embedded Media
  // Field, any older content will automatically update this array as needed.
  // In any case, you should account for the version if you increment it.
  $data['emvideo_comedycentral_version'] = EMVIDEO_COMEDYCENTRAL_DATA_VERSION;

  // We are using oEmbed to retrieve a standard set of data from the provider.
  // You should change the URL as specified by the comedycentral provider.
  // If the comedycentral provider does not support oEmbed, you must remove this
  // section entirely, or rewrite it to use their API.
  // See http://oembed.com/ for for information.

  //@TODO add variable to use either the emfield_request_xml or direct, simplexml
  //$xml = simplexml_load_file('http://media.mtvnservices.com/player/config.jhtml?uri='. $item['value']);
  $response = drupal_http_request('http://media.mtvnservices.com/player/config.jhtml?uri='. $item['value'],
    array('X-Forwarded-For' => '12.13.14.15') // this is a hack to work from the UK
  );

  if ($response->code == 200) {
    $xml = simplexml_load_string($response->data);

    $item = $xml->player->feed->rss->channel->item;
    // <media:group>
    $media = $item->children('http://search.yahoo.com/mrss/');

    $data['thumbnail'] = (string) $media->group->thumbnail->attributes()->url;

  }

  return $data;
}

/**
 *  hook emvideo_PROVIDER_rss
 *
 *  This attaches a file to an RSS feed.
 */
function emvideo_comedycentral_rss($item, $teaser = NULL) {
  if ($item['value']) {
    $file['thumbnail']['filepath'] = $item['data']['thumbnail'];

    return $file;
  }
}

/**
 * hook emvideo_PROVIDER_embedded_link($video_code)
 * returns a link to view the video at the provider's site.
 *  @param $video_code
 *    The string containing the video to watch.
 *  @return
 *    A string containing the URL to view the video at the original provider's site.
 */
function emvideo_comedycentral_embedded_link($video_code) {
  return 'http://www.comedycentral.com/video/'. $video_code;
}

/**
 * The embedded flash displaying the comedycentral video.
 */
function theme_emvideo_comedycentral_flash($item, $width, $height, $autoplay) {
  $output = '';

  if ($item['embed']) {
    $autoplay = $autoplay ? 'true' : 'false';
    $fullscreen = variable_get('emvideo_comedycentral_full_screen', 1) ? 'true' : 'false';
$output = <<<EOD
<embed src='http://media.mtvnservices.com/{$item['value']}' width='$width' height='$height' allowFullscreen='$fullscreen' flashvars='autoPlay=$autoplay' type='application/x-shockwave-flash' wmode='transparent' allowscriptaccess='always' allownetworking='all'></embed>
EOD;

  }
  return $output;
}

/**
 * hook emvideo_PROVIDER_thumbnail
 * Returns the external url for a thumbnail of a specific video.
 *  @param $field
 *    The field of the requesting node.
 *  @param $item
 *    The actual content of the field from the requesting node.
 *  @return
 *    A URL pointing to the thumbnail.
 */
function emvideo_comedycentral_thumbnail($field, $item, $formatter, $node, $width, $height) {
  return $item['data']['thumbnail'];
}

/**
 *  hook emvideo_PROVIDER_video
 *  This actually displays the full/normal-sized video we want, usually on the
 *  default page view.
 *  @param $embed
 *    The video code for the video to embed.
 *  @param $width
 *    The width to display the video.
 *  @param $height
 *    The height to display the video.
 *  @param $field
 *    The field info from the requesting node.
 *  @param $item
 *    The actual content from the field.
 *  @return
 *    The html of the embedded video.
 */
function emvideo_comedycentral_video($embed, $width, $height, $field, $item, $node, $autoplay) {
  $output = theme('emvideo_comedycentral_flash', $item, $width, $height, $autoplay);
  return $output;
}

/**
 *  hook emvideo_PROVIDER_video
 *
 *  This actually displays the preview-sized video we want, commonly for the
 *  teaser.
 *  @param $embed
 *    The video code for the video to embed.
 *  @param $width
 *    The width to display the video.
 *  @param $height
 *    The height to display the video.
 *  @param $field
 *    The field info from the requesting node.
 *  @param $item
 *    The actual content from the field.
 *  @return
 *    The html of the embedded video.
 */
function emvideo_comedycentral_preview($embed, $width, $height, $field, $item, $node, $autoplay) {
  $output = theme('emvideo_comedycentral_flash', $item, $width, $height, $autoplay);
  return $output;
}

/**
 *  Implementation of hook_emfield_subtheme.
 *  This returns any theme functions defined by this provider.
 */
function emvideo_comedycentral_emfield_subtheme() {
    $themes = array(
        'emvideo_comedycentral_flash'  => array(
            'arguments' => array('item' => NULL, 'width' => NULL, 'height' => NULL, 'autoplay' => NULL),
            'file' => 'providers/comedycentral.inc',
            // If you don't provide a 'path' value, then it will default to
            // the emvideo.module path. Obviously, replace 'emcomedycentral' with
            // the actual name of your custom module.
            'path' => drupal_get_path('module', 'emcomedycentral'),
        )
    );
    return $themes;
}
