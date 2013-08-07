<?php

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function brilliant_gallery_cron() {
  // Delete all expired images from the file cache folder brilliant_gallery_pcache
  $last_bg_cron = variable_get('brilliant_gallery_last_cron', 0);
  // brilliant_gallery_cleantmpdir also needs to clear Drupal cache, but we don't want that to happen at every cron, so let's limit BG cron running to once in the cache expiration period.
  if (REQUEST_TIME - $last_bg_cron > brilliant_gallery_get_days_in_seconds(variable_get('brilliant_gallery_cache_duration', 90))) {
    variable_set('brilliant_gallery_last_cron', REQUEST_TIME);
    brilliant_gallery_cleantmpdir();
    brilliant_gallery_clean_table_image_arrays();
  }
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function brilliant_gallery_clean_table_image_arrays() {
  $deletecreatedbeforethistime = REQUEST_TIME - brilliant_gallery_get_days_in_seconds(variable_get('brilliant_gallery_cache_duration', 90));
  //$deletecreatedbeforethistime = date( 'Y-m-d H:i:s', $deletecreatedbeforethistime );
  //echo "DELETE FROM {brilliant_gallery_image_arrays} WHERE datetime < '".$deletecreatedbeforethistime."'"; exit();
  // TODO Please convert this statement to the D7 database API syntax.
  $dbres = db_query("DELETE FROM {brilliant_gallery_image_arrays} WHERE datetime < '" . $deletecreatedbeforethistime . "'");
  //while ($node = db_fetch_object($dbres)) {
  foreach ($dbres as $node) {
    // Perform operations on $node->body, etc. here.
  }
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function brilliant_gallery_cleantmpdir() {
  // Delete files and directories from the file cache directory if their time is up!
  $timenow = REQUEST_TIME;
  $bgcachexpire = brilliant_gallery_get_days_in_seconds(variable_get('brilliant_gallery_cache_duration', 90));
  $timestampexpired = REQUEST_TIME - $bgcachexpire;
  $cachetempdirectory = variable_get('brilliant_gallery_pcache', BRILLIANT_GALLERY_DEFAULT_CACHE_DIR);
  //brilliant_gallery_check_or_create_dir($cachetempdirectory);
  /*
    if ($cachetempdirectory == ''){
    // If there is no cache directory in the files folder, then we need to use the default temp dir
    $cachetempdirectory = file_directory_temp();
    } else {
    $cachetempdirectory = realpath(file_directory_path()) . '/' . $cachetempdirectory;
    }
   */
  $cachetempdirectory = realpath(FILE_DIRECTORY_PATH) . '/' . $cachetempdirectory;

  $GLOBALS['bg_removedcnt'] = 0;
  #watchdog('Brilliant Gal','sakr2: '.$cachetempdirectory);
  #watchdog('Brilliant Gal Cron','in');
  brilliant_gallery_rmdir_recursive($cachetempdirectory, $timestampexpired);

  // Also clear the Drupal cache - otherwise there might be some cached HTML pointing to files that are no longer in the file cache.
  cache_clear_all();

  watchdog('Brilliant Gal', 'Cleared ' . $GLOBALS['bg_removedcnt'] . ' files from the temp directory ' . $cachetempdirectory . ' after ' . variable_get('brilliant_gallery_cache_duration', 90) . ' days. Elapsed time: ' . (REQUEST_TIME - $timenow) . ' seconds.');
}

/*
 * http://nashruddin.com/Remove_Directories_Recursively_with_PHP
 * This function removes a directory and its contents. Use with care; no undo!
 * The function determines if argument is a regular file or a directory. If it is a regular file, it will remove the file. If it is a directory, it will remove its contents first by calling the function itself, and then removes the empty directory.
 * This requires PHP 5 (because of 'scandir').
 */

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function brilliant_gallery_rmdir_recursive($dir, $timestampexpired) {
  #return; // TEMP SAFETY - TO BE CORRECTED LATER!
  // Remove '/' at the end, if any
  $slash = '/';
  if (substr($dir, -1) == '/') {
    $slash = '';
  }
  $files = scandir($dir);
  array_shift($files); // remove '.' from array
  array_shift($files); // remove '..' from array
  foreach ($files as $file) {
    $filename = $file;
    $file = $dir . $slash . $file;
    //watchdog('Brilliant Gal Cron','1: '.$timestampexpired);
    //drupal_set_message('1: '.$timestampexpired);
    if (is_dir($file)) {
      if (substr(strtolower($filename), 0, strlen('bg_picasa_orig_')) == 'bg_picasa_orig_'
      //OR substr(strtolower($filename), 0, strlen('bg_cached_resized_')) == 'bg_cached_resized_'
      ) {
        // Only delete files that start 'bg_picasa_orig_' OR 'bg_cached_resized_'
        //drupal_set_message('folder: '.$filename);
        //watchdog('Brilliant Gal Cron','folder: '.$filename);
        //brilliant_gallery_rmdir_recursive($file, $timestampexpired);
        #drupal_set_message('dir: '.$file.' ('.$timestampexpired.')'); flush();
        @rmdir($file);
      }
    }
    else {
      #unlink($file);
      $suffix = strtolower(brilliant_gallery_get_extension($filename));
      //drupal_set_message('2: '.$filename.'...'.$suffix);
      if (
          $suffix == 'jpg'
          OR $suffix == 'jpeg'
          OR $suffix == 'png'
          OR $suffix == 'gif'
      //OR (strtolower(substr($filename, 0, 3)) == 'bg_cached_resized_' ) // Files like e.g. bg_ffbbaff39504d459ed277714786267b8
      ) {
        # It is a file in the specified BG cache directory
        //drupal_set_message('3: '.$file.'...'.$suffix);
        //watchdog('Brilliant Gal Cron','3: '.$file.'...'.$suffix);
        #watchdog('Brilliant Gal Cron','file: '.$file . '('.@filemtime($file));
        if (@filemtime($file) < $timestampexpired) {
          #drupal_set_message('file: '.$file . '('.filemtime($file).' = '.date('Y-m-d H:i:s',filemtime($file)).') (++'.$GLOBALS['bg_removedcnt'].')');
          $unlinked = unlink($file);
          if ($unlinked) {
            $GLOBALS['bg_removedcnt']++;
          }
        }
      }
    }
  }
}
