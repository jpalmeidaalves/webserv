<?php include "partials/header.php" ?>

<!--hero header-->
<section class="py-7 py-md-0 bg-hero" id="home">
    <div class="container">
        <div class="row vh-md-100">
            <div class="col-md-8 col-sm-10 col-12 mx-auto my-auto text-center">
                <h1 class="heading-black text-capitalize">Requests</h1>
                <p class="lead py-3">This project is part of the 42Porto Common Core Curriculum.</p>
                <button class="btn btn-primary d-inline-flex flex-row align-items-center">
                    See it in action
                    <em class="ml-2" data-feather="arrow-right"></em>
                </button>
            </div>
        </div>
    </div>
</section>

<section>
<div class="container">
        <div class="row">
            <div class="col-md-6 mx-auto text-center">
                <h2 class="heading-black" id="uploads">Uploaded files</h2>
                <p class="text-muted lead">File uploads are handle with POST requests and deleting files with DELETE requests.</p>
            </div>
        </div>
        <div class="row mt-5">

        <?php 
            // $dirPath contain path to directory whose files are to be listed
            $dirPath = "./uploads";
            $files = scandir($dirPath);  
            $count = 0;
            foreach ($files as $file) {
                
                $filePath = $dirPath . '/' . $file;
                if (is_file($filePath)) {

                    if(@is_array(getimagesize($filePath))){
                        $image = $filePath;
                    } else {
                        $image = "./img/file-icon.png";
                    }

                    echo "
                    <div class='col-md-4 pb-4'>
                        <div class='card'>
                            <a href='#'>
                                <img class='card-img-top img-raised' src='{$image}' alt='Blog 1'>
                            </a>
                            <div class='card-body'>
                                <a href='#' class='card-title mb-2'><h5>{$filePath}</h5></a>
                                <p class='card-text'><button type='button' data-toggle='modal' data-target='#upload-{$count}' class='btn btn-danger btn-sm'>delete</button></p>
                            </div>



                            <!-- Modal -->
                            <div class='modal fade' id='upload-{$count}' tabindex='-1' role='dialog' aria-hidden='true'>
                            <div class='modal-dialog' role='document'>
                                <div class='modal-content'>
                                <div class='modal-header'>
                                    <h5 class='modal-title'>Delete file {$filePath}?</h5>
                                    <button type='button' class='close' data-dismiss='modal' aria-label='Close'>
                                    <span aria-hidden='true'>&times;</span>
                                    </button>
                                </div>
                                <div class='modal-body'>
                                    This action is irreversible, are you sure you want to continue?
                                </div>
                                <div class='modal-footer'>
                                    <button type='button' class='btn btn-secondary' data-dismiss='modal'>Cancel</button>
                                    <a href='delete.php?name={$filePath}' class='btn btn-danger'>Delete</a>
                                </div>
                                </div>
                            </div>
                            </div>


                        </div>
                    </div>
                    ";
                    $count++;
                }
            }

            


        ?>

        </div>
            <?php
        if ($count == 0) {
                echo "<p class='text-center'>No files in the <code>/uploads</code> directory!</p>";
            }
            ?>
        <div class="row mt-6">
            <div class="col-md-6 mx-auto text-center">
            <form action="upload.php" method="post" enctype="multipart/form-data">
                Select image to upload:
                <input type="file" name="files[]" id="files" multiple />
                <input type="submit" value="Upload Image" name="submit">
            </form>
            </div>
        </div>
    </div>
</section>

<?php include "partials/footer.php" ?>