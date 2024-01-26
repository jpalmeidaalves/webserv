<?php header('HTTP/1.1 200 OK');



// header('Content-Type: text/plain');


// header('Content-Type: text/another');




header('Access-Control-Allow-Origin: *');

header('Access-Control-Allow-Methods: GET, POST');

header("Access-Control-Allow-Headers: X-Requested-With");

header("custom header: hello world!");

?>


<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
    
<h1>hello <?php echo isset($_GET["name"]) ? $_GET["name"] : "world"; ?></h1>

<?php
var_dump($_GET);
?>

<?php var_dump($_POST); ?>

<?php file_get_contents('php://input'); ?>

</body>
</html>