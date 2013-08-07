<?php

function testext($filename) {
  if (substr(strtolower($filename), -4, 4) == ".jpg" or substr(strtolower($filename), -5, 5) == ".jpeg" or substr(strtolower($filename), -4, 4) == ".gif" or substr(strtolower($filename), -4, 4) == ".png") {
    return true;
  }
  // Otherwise return false.
  return false;
}

function load_dir_to_array($absolpath, $imagewidth, $fullresolutionmaxwidth, $brilliant_gallery_sort, $imagecrop, $maximumnumbertoshow = '') {
  #watchdog('Brilliant Gal','imgw: '.$imagewidth.'/'.$imagecrop);
  # Load Directory Into Array
  $poct = -1;
  $retval_dimensions = array();
  #$handle            = @opendir($absolpath);
  $actualpath = realpath(FILE_DIRECTORY_PATH) . $absolpath;
  #watchdog('Brilliant Gal','absol3: '.$actualpath);
  $handle = @opendir($actualpath);
  $imagemaxh = 0;

  // Load the directory into an array first.
  $filearray = array();
  while ($file = @readdir($handle)) {
    $testending = strtolower(substr($file, -4, 4));
    if (!testext($file) and strtolower($testending) <> '.mpg' and strtolower($testending) <> '.swf' and strtolower($testending) <> '.mov' and strtolower($testending) <> '.avi') {
      continue;
    }
    $filearray[] = $file;
  }
  @closedir($handle);

  if (empty($filearray)) {
    watchdog('Brilliant Gal', 'No displayable images in ' . $absolpath . '!');
  }
  else {
    foreach ($filearray as $file) {
      # Index of a real image or something we are going to display.
      $poct += 1;
      $retval_dimensions[$poct]['file'] = $file;
      $retval_dimensions[$poct]['imgcrop'] = $imagecrop;

      # Is image horizontally or vertically oriented?
      #$temp = getimagesize($absolpath .'/'. $file);
      $temp = getimagesize($actualpath . '/' . $file);
      if ($temp === false) {
        continue;
      }
      if ((($temp[0] - $temp[1]) >= 0) OR $maximumnumbertoshow == 1) {
        // This is a horizontal image.
        // Treat single images just as horizontal images (no need to fit their height to the height of horizontals)! The specified width of an image is authoritative for both horizontal images and also vertical if they show alone.
        #if ($imagecrop == 'yes' and $maximumnumbertoshow <> 1){
        if ($imagecrop == 'yes') {
          $retval_dimensions[$poct]['imgw'] = $imagewidth;
          $retval_dimensions[$poct]['imgh'] = $imagewidth;
        }
        else {
          $retval_dimensions[$poct]['imgw'] = $imagewidth;
          $retval_dimensions[$poct]['imgh'] = round(($temp[1] / $temp[0]) * $imagewidth);
          #watchdog('Brilliant Gal','1imgw: '.$imagewidth.'/'.$imagecrop);
        }
        $retval_dimensions[$poct]['imgwbig'] = $fullresolutionmaxwidth;
        $retval_dimensions[$poct]['imghbig'] = round(($temp[1] / $temp[0]) * $fullresolutionmaxwidth);
      }
      else {
        // This is a vertical image
        if ($imagecrop == 'yes') {
          $retval_dimensions[$poct]['imgw'] = round(($temp[0] / $temp[1]) * $imagewidth);
          $retval_dimensions[$poct]['imgh'] = $imagewidth;
        }
        else {
          $retval_dimensions[$poct]['imgw'] = round(($temp[0] / $temp[1]) * ($temp[0] / $temp[1]) * $imagewidth);
          $retval_dimensions[$poct]['imgh'] = round(($temp[0] / $temp[1]) * $imagewidth);
          #watchdog('Brilliant Gal','2imgw: '.$imagewidth.'/'.$imagecrop);
          #watchdog('Brilliant Gal','2imgw: '.$temp[0].'-'.$temp[1]);
        }
        $retval_dimensions[$poct]['imgwbig'] = round(($temp[0] / $temp[1]) * ($temp[0] / $temp[1]) * $fullresolutionmaxwidth);
        $retval_dimensions[$poct]['imghbig'] = round(($temp[0] / $temp[1]) * $fullresolutionmaxwidth);
      }
      // In $imagemaxh collect the maximum vertical size of the gallery.
      if ($imagemaxh < $retval_dimensions[$poct]['imgh']) {
        $imagemaxh = $retval_dimensions[$poct]['imgh'];
      }
    }
  }

  if ($brilliant_gallery_sort == '1') {
    @sort($retval_dimensions);
  }
  else {
    shuffle($retval_dimensions);
  }
  return array($retval_dimensions, $imagemaxh);
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function brilliant_gallery_brokenimage($msg, $width = 150, $height = 30) {
  header('Content-type: image/jpeg');
  $im = imagecreatetruecolor($width, $height);
  $bgc = imagecolorallocate($im, 0, 0, 0);
  $tc = imagecolorallocate($im, 255, 255, 255);
  imagefilledrectangle($im, 0, 0, 150, 30, $bgc);
  imagestring($im, 1, 5, 5, $msg, $tc);
  imagejpeg($im);
  exit();
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function brilliant_gallery_image_properties_set($bgimgproperties_array) {
  // Binds image property array with its hash that is present in the cached file name and in the URL.

  $bgimgproperties_hash = md5(serialize($bgimgproperties_array));

  // Let's keep the array in cache. Check if it is in cache. If not, write it.
  // TODO Please convert this statement to the D7 database API syntax.
  $countie = db_query("SELECT COUNT(*) FROM {brilliant_gallery_image_arrays} WHERE hash = '" . $bgimgproperties_hash . "'")->fetchField();
  if ($countie == 0) {
    // Run an insert
    //echo 1;
    $fields = new stdClass();
    $fields->hash = $bgimgproperties_hash;
    $fields->array = serialize($bgimgproperties_array);
    //$fields->datetime = date('Y-m-d H:i:s', REQUEST_TIME);
    $fields->datetime = REQUEST_TIME;
    drupal_write_record("brilliant_gallery_image_arrays", $fields);
  }
  else {
    // Run an update
    $change = array(
      //'datetime' => date('Y-m-d H:i:s', REQUEST_TIME),
      'datetime' => REQUEST_TIME,
      'hash' => $bgimgproperties_hash,
    );
    drupal_write_record('brilliant_gallery_image_arrays', $change, 'hash');
  }

  return $bgimgproperties_hash;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function brilliant_gallery_image_properties_get($bgimgproperties_hash) {

  // TODO Please convert this statement to the D7 database API syntax.
  $bgimgproperties_array = db_query("SELECT array FROM {brilliant_gallery_image_arrays} WHERE hash = '" . $bgimgproperties_hash . "' LIMIT 1")->fetchField();
  $bgimgproperties_array = unserialize($bgimgproperties_array);

  return $bgimgproperties_array;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function brilliant_gallery_get_extension($imagename) {
  $imagename = explode(".", $imagename);
  $extension = strtolower(array_pop($imagename));
  return $extension;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function brilliant_gallery_get_days_in_seconds($days) {
  return $days * 24 * 3600;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function brilliant_gallery_deliver_image() {

  $queryexplode = explode('/', $_GET['q']);
  //$bgimgproperties_hash_position = sizeof($queryexplode)+1;
  $bgimgproperties_hash = array_pop($queryexplode); // Get the last item in the query.
  $bgimgproperties_hash = substr($bgimgproperties_hash, strlen('bg_cached_resized_')); // Remove prefix.
  //return (base64_decode($bgimgproperties_hash));
  //include 'image.inc';
  // The file name contains a hash token that corresponds to the image parameter array; this relation is in cache.
  $bgimgproperties_hash_array = explode('.', $bgimgproperties_hash);
  $bgimgproperties_hash_array = $bgimgproperties_hash_array[0];
  $bgimgproperties_array = brilliant_gallery_image_properties_get($bgimgproperties_hash_array);
  //print_r($bgimgproperties_array); flush(); exit();
  if (!is_array($bgimgproperties_array)) {
    watchdog('Brilliant Gal', 'Wrong image: ' . $bgimgproperties_hash);
    return;
  }
  foreach ($bgimgproperties_array as $key => $val) {
    //$showthis .= $val.' -- ';
    $_GET[$key] = $val;
  }


  /* Check for bad URL inputs */
  $urlpath = $_GET['imgp'];
  //brilliant_gallery_brokenimage($urlpath.' :0: '.time(),1900,500); exit();
  if (sizeof($_GET) > 5 ||
      strpos($urlpath, "://") !== false ||
      strpos($urlpath, "..") !== false ||
      preg_match('/\D/', ($_GET['imgw'] . $_GET['imgh'])) > 0 ||
      ($_GET['imgw'] + $_GET['imgh']) < 10 ||
      ($_GET['imgw'] + $_GET['imgh']) > 20000
  ) {
    header("HTTP/1.0 404 Not Found");
    exit();
  }

  /*
    drupalize();
    function drupalize() {
    while (!@stat('./includes/bootstrap.inc')) {
    chdir('..');
    }
    #module_load_include('/includes/bootstrap.inc', 'image', 'includes/bootstrap');
    require_once './includes/bootstrap.inc';
    require_once './includes/file.inc';
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL); // See http://drupal.org/node/211378#comment-924059
    #drupal_bootstrap(DRUPAL_BOOTSTRAP_DATABASE);
    #drupal_cron_run();
    }
   */
  $imagepath = realpath(FILE_DIRECTORY_PATH . $urlpath);
  #watchdog('Brilliant Gal','imgp: '.$imagepath);
  // Crucial - to suppress Devel (if installed and enabled) output appearing in the generated XML!
  $GLOBALS['devel_shutdown'] = FALSE;

  #chdir ('../../../../');
  #module_load_include('/includes/bootstrap.inc', 'image', 'includes/bootstrap');
  #module_load_include('./../../../../includes/bootstrap.inc', 'image', '');
  #drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  #if ( $_SERVER['SERVER_ADDR'] == '64.13.192.90' ) {
  /*
    if (variable_get('brilliant_gallery_cache', 'd') == 'f') {
    #echo '.....................' . $_SERVER['SERVER_ADDR'];
    #drupal_set_message('cachetype1 '.variable_get('brilliant_gallery_cache', 'd'));
    #watchdog('Brilliant Gal', '1 '.variable_get('brilliant_gallery_cache'));
    $my_data = resizeimage_wrapper_filecache();
    }
    else {
    #drupal_set_message('cachetype2 '.variable_get('brilliant_gallery_cache', 'd'));
    #watchdog('Brilliant Gal', '2 '.variable_get('brilliant_gallery_cache'));
    $my_data = resizeimage_wrapper_dbcache();
    }
   */
  //brilliant_gallery_brokenimage($bgcacheid.' :g: '.time(),1900,500); exit();
  $my_data = resizeimage_wrapper_dbcache(FALSE, $imagepath, $bgimgproperties_hash);

  #echo '....'. sess_read('vacilando');
  $imgsize = @getimagesize($imagepath);
  # http://be.php.net/getimagesize
  $head = "Content-type: {$imgsize['mime']}";
  header($head);
  //header($my_data[0]);
  echo $my_data;
  // IMPORTANT to exit() - otherwise some process after BG adds strings and breaks the image!
  exit();
  /*
    function resizeimage_wrapper_filecache() {
    $bgcacheid = 'bg_'. md5($_GET['imgp'] . $_GET['imgw'] . $_GET['imgh']);
    #echo '. 0.... ';
    $bgcachexpire = variable_get('brilliant_gallery_cache_duration',90) * 24 * 3600; // Cache expiration time in days.
    // Tested that both relative (eg sites/all/files/cache) and absolute (eg /home/data/tmp) tmp path settings work OK here.
    $cachetemp = variable_get('brilliant_gallery_pcache', BRILLIANT_GALLERY_DEFAULT_CACHE_DIR);
    $cachedfile = $cachetemp .'/'. $bgcacheid;
    $fileexists = false;
    $fileexists = file_exists($cachedfile);
    $timenow = time();
    $lastchanged = $timenow;
    $lastchanged = @filemtime($cachedfile);
    $fileexpired = false;
    if ($timenow - $lastchanged > $bgcachexpire){
    $fileexpired = true;
    // If the image is expired, we need to actively delete it, for the case that it was removed / hidden by the owner.
    @unlink($cachedfile);
    }
    if (!$fileexists or $fileexpired) {
    #echo '. 1.... ';
    // Cache file does not exist or is too old.
    #$my_data = resizeimage($_GET['imgp'], $_GET['imgw'], $_GET['imgh']);
    $my_data = resizeimage($_GET['imgp'], $_GET['imgw'], $_GET['imgh'], $_GET['imgcrop']);
    // Now put $my_data to cache!
    $fh = fopen($cachedfile, "w+");
    fwrite($fh, $my_data);
    fclose($fh);
    $my_data = unserialize($my_data);
    }
    else {
    #echo '. 2.... ';
    // Cache file exists.
    $my_data = unserialize(file_get_contents($cachedfile));
    }
    return $my_data;
    }
   */

  /*
    brilliant_gallery_brokenimage($showthis,500,500); exit();
   */

  /*
    $result = '';

    // $bgimgproperties_hash looks like d48a36adcaff5b04858e6ae8e8d2b81a.bgimg -- let's lose the extension.
    $bgimgproperties_hash = explode('.',$bgimgproperties_hash);
    $bgimgproperties_hash = $bgimgproperties_hash[0];

    drupal_set_message('hashed array: '.$bgimgproperties_hash);

    $result .= $bgimgproperties_hash;
   */

  //return $result;
  return;
}

function resizeimage_wrapper_dbcache($reset = FALSE, $imagepath, $bgimgproperties_hash) {
  #global $user;
  #$userId = $user->uid;
  $bgcachexpire = brilliant_gallery_get_days_in_seconds(variable_get('brilliant_gallery_cache_duration', 90)); // Cache expiration time in days.
  //$bgcacheid = 'bg_' . md5($_GET['imgp'] . $_GET['imgw'] . $_GET['imgh']);
  //brilliant_gallery_brokenimage($bgcacheid.' :0: '.time(),1900,500); exit();
  #echo $bgcacheid;
  static $my_data;
  #echo '0.... ';

  /*
    $reset = true;
    if (!isset($my_data) || $reset) {
    if (!$reset and ($cache = cache_get($bgcacheid)) and !empty($cache->data)) {
    #$my_data = $cache->data; echo '-1.... ' . $my_data;
    // Crucial to unserialize for the immediate use!
    $my_data = unserialize($cache->data);
    #echo $my_data;
    //brilliant_gallery_brokenimage($bgcacheid.' :1: '.time(),1900,500); exit();
    }
    else {
    // Do your expensive calculations here, and populate $my_data
    // with the correct stuff..
    #$my_data = resizeimage($_GET['imgp'], $_GET['imgw'], $_GET['imgh']);
    //$my_data = resizeimage($_GET['imgp'], $_GET['imgw'], $_GET['imgh'], $_GET['imgcrop'],$imagepath);
    #echo ' -2.... ' . $bgcachexpire . ' // ' . $my_data;
    # For some reason I could not use: mysql_escape_string($my_data)
    #cache_set($bgcacheid, 'cache', time() + $bgcachexpire, $my_data);
    cache_set($bgcacheid, $my_data, 'cache', time() + $bgcachexpire);
    //brilliant_gallery_brokenimage($_GET['imgp']. $_GET['imgw']. $_GET['imgh']. $_GET['imgcrop'].' :3: '.time(),1900,500); exit();
    # FOR DRUPAL6 MUST USE:
    #cache_set($bgcacheid,  $my_data, time() + $bgcachexpire); # For some reason I could not use: mysql_escape_string($my_data)
    // Crucial to unserialize for the immediate use!
    $my_data = unserialize($my_data);
    }
    }
   */


  $pcache = variable_get('brilliant_gallery_pcache', BRILLIANT_GALLERY_DEFAULT_CACHE_DIR);
  //brilliant_gallery_check_or_create_dir($pcache);
  $foqen = FILE_DIRECTORY_PATH . '/' . $pcache . '/bg_cached_resized_' . $bgimgproperties_hash;
  $lastchanged = @filemtime($foqen); // Last file modification time, or FALSE on error.
  /*
    global $user; $userId = $user->uid;
    if ($userId == 1){
    drupal_set_message('pic '.($timenow - $lastchanged).' > '.$bgcachexpire.' ... '.$foqen);
    }
   */
  #if ($_GET['fetchnow']==1){ $lastchanged = FALSE; } // Debugging!
  if ($lastchanged === FALSE OR (REQUEST_TIME - $lastchanged > $bgcachexpire)) {
    // If the image is expired, we need to actively delete it, for the case that it was removed / hidden by the owner.
    @unlink($foqen);
    /*
      global $user; $userId = $user->uid;
      if ($userId == 1){
      drupal_set_message('in!');
      }
     */
    ///*
    $my_data = resizeimage($_GET['imgp'], $_GET['imgw'], $_GET['imgh'], @$_GET['imgcrop'], $imagepath);
    $image = $my_data;
    //*/
    #watchdog('bg', $imager->code.' ... '.$imgurl.' =imgurl');
    // It happens that the file size is 0 (image not fetched). In such case, don't write it.
    if (strlen($image) > 0) {
      $fp = fopen($foqen, 'w');
      #drupal_set_message($foqen.'<br>');
      #watchdog('bg', $foqen.' ...ok<br>');
      fwrite($fp, $image);
      fclose($fp);
    }
  }


  return $my_data;
}

#function resizeimage($imgp, $imgw, $imgh) {

function resizeimage($imgp, $imgw, $imgh, $imgcrop, $imagepath) {
  #$imagepath = base64_decode($imgp);
  #echo '.... ' . base64_decode( $imgp );
  #flush();die(' stop!');
  //global $imagepath;

  $suffix = strtolower(substr($imagepath, -4));
  //$imgsize = @getimagesize($imagepath);
  # http://be.php.net/getimagesize
  //$head = "Content-type: {$imgsize['mime']}";
  if ($suffix == ".gif") {
    #$head = "Content-type: image/gif";
    $img = @imagecreatefromgif($imagepath);
    if (!$img) {
      brilliant_gallery_brokenimage("Error loading GIF", $imgw, $imgh);
    }
  }
  else if ($suffix == ".jpg" or $suffix == "jpeg") {
    #$head = "Content-type: image/jpeg";
    //brilliant_gallery_brokenimage($imagepath.' :1: '.time(),1900,500); exit();
    $img = @imagecreatefromjpeg($imagepath);
    if (!$img) {
      brilliant_gallery_brokenimage("Error loading JPG", $imgw, $imgh);
    }
  }
  else if ($suffix == ".png") {
    #$head = "Content-type: image/png";
    $img = @imagecreatefrompng($imagepath);
    if (!$img) {
      brilliant_gallery_brokenimage("Error loading PNG", $imgw, $imgh);
    }
  }
  # Resize the image
  $src_h = ImageSY($img);
  $src_w = ImageSX($img);
  #$dst_img = imagecreatetruecolor($imgw, $imgh);
  #imagecopyresampled($dst_img, $img, 0, 0, 0, 0, $imgw, $imgh, $src_w, $src_h);
  $dst_img = 0;
  if ($imgcrop == 'yes') {
    if ($src_h > $src_w) {
      // portrait
      $dst_img = imagecreatetruecolor($imgh, $imgh);
      imagecopyresampled($dst_img, $img, 0, 0, 0, ($src_h - $src_w) / 2, $imgh, $imgh, $src_w, $src_w);
    }
    else {
      // landscape
      $dst_img = imagecreatetruecolor($imgw, $imgw);
      imagecopyresampled($dst_img, $img, 0, 0, ($src_w - $src_h) / 2, 0, $imgw, $imgw, $src_h, $src_h);
    }
  }
  else {
    $dst_img = imagecreatetruecolor($imgw, $imgh);
    imagecopyresampled($dst_img, $img, 0, 0, 0, 0, $imgw, $imgh, $src_w, $src_h);
  }
  $img = $dst_img;
  //imageinterlace($img, 1);
  //imagecolortransparent($img);
  ob_start();
  if ($suffix == ".gif") {
    Imagegif($img);
  }
  else if ($suffix == ".jpg" or $suffix == ".jpeg") {
    Imagejpeg($img, NULL, 90);
  }
  else if ($suffix == ".png") {
    Imagepng($img, NULL, 9);
  }
  $result = ob_get_clean();
  #ImageDestroy($img);
  //$result = serialize(array($head, base64_encode($result)));
  $result = $result;
  return $result;
}

function replace_brilliant_gallery_tags($str) {
  # Old format - still supported
  # [bg|path/to/gallery_folder|colcountoverride|widthoverride|sortorrandomoverride|maximumnumbertoshow|colouroverride|beginfromoverride|caption-yes-no-text]
  # New format - allows multiline tags (strips out HTML), works with parameter = value attribution (position independent!)
  #
  #
  #

  $matchlink = '';
  $orig_match = '';
  $matches = '';
  preg_match_all("/(\[)bg(\|)[^\]]*(\])/s", $str, $matches);
  foreach ($matches[0] as $match) {
    $omatch = $match;
    $orig_match[] = $omatch;
    $match = substr($match, 1, (strlen($match) - 2));

    // Remove HTML tags
    $match = strip_tags($match);

    // Create an array of parameter attributions
    $match = explode("|", $match);
    #watchdog('Brilliant Gal', 'Got the new arrayx: '.vacilando_echo_array($match));
    $allowed_params = brilliant_gallery_get_allowed_params();

    // Remove enclosing spaces and get rid of empty parameter attributions.
    $newmatch = array(); // Collect for the legacy style $match array.
    $newgenmatch = array();
    $isnewgettag = true;
    foreach ($match as $val) {
      $tmp = trim($val);
      if ($tmp <> '') {
        $tmp2 = explode(' = ', $tmp);
        #watchdog('Brilliant Gal', $tmp.' // '.sizeof($tmp2).' ('.$tmp2[0].')');
        if (sizeof($tmp2) == 2) {
          // It's possibly a new generation tag
          $tmp2[0] = strtolower(trim($tmp2[0]));
          $tmp2[1] = trim($tmp2[1]);
          // Check if it uses a valid parameter name.
          // The value MAY be none here, to allow re-setting some parameters.
          if (in_array($tmp2[0], $allowed_params)) {
            $newgenmatch[$tmp2[0]] = $tmp2[1];
          }
          else {
            $msg = 'Parameter ' . $tmp2[0] . ' is invalid!';
            #drupal_set_message($msg);
            watchdog('Brilliant Gal', $msg);
          }
        }
        else {
          // Takes care of compatibility with old-style BG tags (one line, no attribution).
          // If one or more of the parameters does not use attribution, the whole tag is treated as an old generation one. Except for 'bg', which is not an attribution parameter.
          if ($tmp <> 'bg') {
            $isnewgettag = false;
          }
          $newmatch[] = $tmp;
        }
      }
    }

    if (!$isnewgettag) {
      // Plain old tag
      $match = $newmatch;
    }
    else {
      // OK, we've got the new generation params in $newgenmatch
      // The order of params to feed into render_brilliant_gallery() is the same as with the old-style tag.
      $match = array();
      foreach ($newgenmatch as $key => $val) {
        $match[array_search($key, $allowed_params)] = $val; // E.g. 'location = myalbum' will become $match[1] = 'myalbum'
        /*
          if (arg(1) == 264402){
          #drupal_set_message('...');
          #var_dump($newgenmatch);
          #var_dump($match);
          echo array_search($key,$allowed_params)." $key // $val\n";
          }
         */
      }
    }

    // If we are using thumbshowbyname and there is just 1 image, force thumbmaxshow to 1 (prevents BG from adjusting the height as it does for a set of images).
    if (@$match[13] <> '' AND strpos(@$match[13], ',') === FALSE) {
      $match[5] = 1;
    }

    #watchdog('Brilliant Gal', 'Got the array: '.vacilando_echo_array($match));
    #watchdog('Brilliant Gal', 'Got the new array: '.vacilando_echo_array($newgenmatch));
    // Cache the result if it comes from a non-random tag
    $bgcachexpire = brilliant_gallery_get_days_in_seconds(variable_get('brilliant_gallery_cache_duration', 90)); // Cache expiration time in days.
    $mbgtag = md5($omatch);
    #########x TEMPORARILY SET TO FALSE!!
    if ($cache = cache_get('bg_gallery_table_' . $mbgtag) and !empty($cache->data)) {
      #if (false) {
      $galhere = $cache->data;
    }
    else {
      # render_brilliant_gallery takes parameters in a fixed order:
      # 1 path/to/gallery_folder
      # 2 colcountoverride
      # 3 widthoverride
      # 4 sortorrandomoverride
      # 5 maximumnumbertoshow
      # 6 colouroverride
      # 7 beginfromoverride
      # 8 caption-yes-no-text
      #watchdog('Brilliant Gal', 'before: '.vacilando_echo_array($match));
      $galhere = render_brilliant_gallery($match);
      cache_set('bg_gallery_table_' . $mbgtag, $galhere, 'cache', brilliant_gallery_get_time_this_many_seconds_from_now($bgcachexpire));
    }

    $matchlink[] = $galhere;
  }
  $str = str_replace($orig_match, $matchlink, $str);
  return $str;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function brilliant_gallery_get_time_this_many_seconds_from_now($period) {
  $period = REQUEST_TIME + $period;
  return $period;
}
