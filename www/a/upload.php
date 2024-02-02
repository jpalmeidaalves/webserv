<?php

header("custom header: hello world!");

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

var_dump($_FILES);

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
    //Setup our new file path
    $newFilePath = "./uploads/" . $_FILES['files']['name'][$i];

    //Upload the file into the temp dir
    if(move_uploaded_file($tmpFilePath, $newFilePath)) {

      echo 'uploaded file!';

    } else {
      echo 'failed to upload!';
    }
  }
}



// $target_dir = "./uploads/";
// echo $_FILES["fileToUpload"]["name"];
// $target_file = $target_dir . basename($_FILES["fileToUpload"]["name"]);
// $uploadOk = 1;
// $imageFileType = strtolower(pathinfo($target_file,PATHINFO_EXTENSION));
// // Check if image file is a actual image or fake image
// if(isset($_POST["submit"])) {
//   $check = getimagesize($_FILES["fileToUpload"]["tmp_name"]);
//   if($check !== false) {
//     echo "File is an image - " . $check["mime"] . ".";
//     $uploadOk = 1;
//   } else {
//     echo "File is not an image.";
//     $uploadOk = 0;
//   }
// }

// if (move_uploaded_file($_FILES['fileToUpload']['tmp_name'], $target_dir . $_FILES['fileToUpload']['name'])) {
//   echo 'Received file' . $_FILES['fileToUpload']['name'] . ' with size ' . $_FILES['fileToUpload']['size'];
// } else {
//   echo 'Upload failed!';

//   var_dump($_FILES['fileToUpload']['error']);
// }
?>
