<?php 
include "partials/header.php";

?>

<section class="py-7">
    <div class="container">
        <h1>Contact Us</h1>

        <form action="/contact.php" method="post" class="mb-4">
        <div class="form-group">
            <label for="name">Name</label>
            <input class="form-control" id="name" name="name" placeholder="Name">
        </div>
        <div class="form-group">
            <label for="message">Message</label>
            <textarea class="form-control" name="message" id="message" placeholder="Message"></textarea>
        </div>
            <input type="submit" name="submit">
        </form>

        <?php
        
        if ($_SERVER["REQUEST_METHOD"] == "POST") {
            var_dump($_POST);
        }
        ?>
    </div>

</section>

<?php include "partials/footer.php" ?>