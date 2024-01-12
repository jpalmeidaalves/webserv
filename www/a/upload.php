<p>PATH_INFO: <?php echo isset($_SERVER[ "PATH_INFO" ]) ? $_SERVER[ "PATH_INFO" ] : ""; ?></p>
<p>CONTENT_TYPE: <?php echo isset($_SERVER[ "CONTENT_TYPE" ]) ? $_SERVER[ "CONTENT_TYPE" ] : ""; ?></p>
<p>CONTENT_TYPE: <?php echo isset($_SERVER[ "CONTENT_TYPE" ]) ? $_SERVER[ "CONTENT_TYPE" ] : ""; ?></p>
<p>QUERY_STRING: <?php echo isset($_SERVER[ "QUERY_STRING" ]) ? $_SERVER[ "QUERY_STRING" ] : ""; ?></p>

<p>query name: <?php echo isset($_GET["name"]) ? $_GET["name"] : ""; ?></p>
<p>query age: <?php echo isset($_GET["age"]) ? $_GET["age"] : ""; ?></p>

<p>post test: <?php echo isset($POST["test"]) ? $POST["test"] : ""; ?></p>
<p>post name: <?php echo isset($POST["name"]) ? $POST["name"] : ""; ?></p>



<?php var_dump($_POST); ?>