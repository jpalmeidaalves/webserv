<?php include "./partials/header.php" ?>

<!--hero header-->
<section class="py-7 py-md-0 bg-hero" id="home">
    <div class="container">
        <div class="row pt-8 pb-5">
            <div class="col-md-8 col-sm-10 col-12 mx-auto my-auto text-center">
                <h1 class="heading-black text-capitalize">Welcome to webserv</h1>
                <p class="lead py-3">This project is part of the 42Porto Common Core Curriculum.</p>
                <a href="/requests.php"> 
                    <button class="btn btn-primary d-inline-flex flex-row align-items-center">
                        See it in action
                        <em class="ml-2" data-feather="arrow-right"></em>
                    </button>
                </a>
            </div>
        </div>
    </div>
</section>

<!-- features section -->
<section class="pt-6 pb-7" id="features">
    <div class="container">
        <div class="row">
            <div class="col-md-6 mx-auto text-center">
                <h2 class="heading-black">Our webserver offers everything you need.</h2>
                <p class="text-muted lead">We can serve your sites as nobody</p>
            </div>
        </div>
        <div class="row mt-5">
            <div class="col-md-10 mx-auto">
                <div class="row feature-boxes">
                    <div class="col-md-6 box">
                        <div class="icon-box box-primary">
                            <div class="icon-box-inner">
                                <span data-feather="edit-3" width="35" height="35"></span>
                            </div>
                        </div>
                        <h5>High performance server!</h5>
                        <p class="text-muted">Proficiently adept at seamlessly juggling an extensive array of requests
                             simultaneously, ensuring efficient management and timely resolution.</p>
                    </div>
                    <div class="col-md-6 box">
                        <div class="icon-box box-success">
                            <div class="icon-box-inner">
                                <span data-feather="monitor" width="35" height="35"></span>
                            </div>
                        </div>
                        <h5>Faster than ever!</h5>
                        <p class="text-muted">Capable of promptly addressing and swiftly responding to all incoming requests in real-time,
                             ensuring seamless and immediate assistance.</p>
                    </div>
                    <div class="col-md-6 box">
                        <div class="icon-box box-danger">
                            <div class="icon-box-inner">
                                <span data-feather="layout" width="35" height="35"></span>
                            </div>
                        </div>
                        <h5>Easy to use and configure</h5>
                        <p class="text-muted">Simple configuration and instalation unlike Nginx!</p>
                    </div>
                    <div class="col-md-6 box">
                        <div class="icon-box box-info">
                            <div class="icon-box-inner">
                                <span data-feather="globe" width="35" height="35"></span>
                            </div>
                        </div>
                        <h5>Available globally</h5>
                        <p class="text-muted">Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum in
                            nisi commodo, tempus odio a, vestibulum nibh.</p>
                    </div>
                </div>
            </div>
        </div>
    </div>
</section>

<?php include "partials/footer.php" ?>