<?php

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

var_dump($_GET);

$config['allowed_types'] = '*';


// $files = array_filter($_FILES['files']['name']); //something like that to be used before processing files.

// Count # of uploaded files in array
$total = count($_FILES['files']['name']);

// Loop through each file
for( $i=0 ; $i < $total ; $i++ ) {

  //Get the temp file path
  $tmpFilePath = $_FILES['files']['tmp_name'][$i];

  //Make sure we have a file path
  if ($tmpFilePath != ""){
    // TODO get uploaded file path here
    //Setup our new file path
    $newFilePath = $_GET["upload_path"] . $_FILES['files']['name'][$i];

    //Upload the file into the temp dir
    if(move_uploaded_file($tmpFilePath, $newFilePath)) {

      echo '<h1>🚀 Files uploaded to!' . $newFilePath . '</h1>';

    } else {
      echo '<h1>upload failed!</h1>';
    }
  }
}


?>
