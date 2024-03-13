<?php include "./partials/header.php" ?>

<!--hero header-->
<section class="py-7 py-md-0 bg-hero" id="home">
    <div class="container">
        <div class="row vh-md-100">
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
        <!-- <div class="row mt-6">
            <div class="col-md-6 mr-auto">
                <h2>Knight is more than just a page builder.</h2>
                <p class="mb-5">Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla convallis pulvinar vestibulum. Donec eleifend, sem sed dictum mattis, turpis purus placerat eros.</p>
                <a href="#" class="btn btn-light">
                    Try the live demo
                </a>
            </div>
            <div class="col-md-5">
                <div class="slick-about">
                    <img src="img/blog-1.jpg" class="img-fluid rounded d-block mx-auto" alt="Work 1"/>
                    <img src="img/blog-2.jpg" class="img-fluid rounded d-block mx-auto" alt="Work 2"/>
                    <img src="img/blog-3.jpg" class="img-fluid rounded d-block mx-auto" alt="Work 3"/>
                </div>
            </div>
        </div> -->
    </div>
</section>

<!--pricing section-->
<!-- <section class="py-7 bg-dark section-angle top-right bottom-right" id="pricing">
    <div class="container">
        <div class="row">
            <div class="col-md-6 mx-auto text-center">
                <h2 class="text-white heading-black">Choose your pricing plan.</h2>
                <p class="text-light lead">Simply pricing - 7 Days free trial</p>
            </div>
        </div> -->
        <!--pricing tables-->
        <!-- <div class="row pt-5 pricing-table">
            <div class="col-12 mx-auto">
                <div class="card-deck pricing-table">
                    <div class="card">
                        <div class="card-body">
                            <h3 class="card-title pt-3">Personal</h3>
                            <h2 class="card-title text-primary mb-0 pt-4">$59</h2>
                            <div class="text-muted font-weight-medium mt-2">per month</div>
                            <ul class="list-unstyled pricing-list">
                                <li>1 user</li>
                                <li>10 websites</li>
                                <li>Access to premium templates</li>
                                <li>Basic support</li>
                            </ul>
                            <a href="#" class="btn btn-primary">
                                Start free trial
                            </a>
                        </div>
                    </div>
                    <div class="card">
                        <div class="card-body">
                            <h3 class="card-title pt-3">Agency</h3>
                            <h2 class="card-title text-info mb-0 pt-4">$159</h2>
                            <div class="text-muted font-weight-medium mt-2">per month</div>
                            <ul class="list-unstyled pricing-list">
                                <li>2-15 users</li>
                                <li>50 websites</li>
                                <li>Access to premium templates</li>
                                <li>Priority support</li>
                            </ul>
                            <a href="#" class="btn btn-info">
                                Start free trial
                            </a>
                        </div>
                    </div>
                    <div class="card">
                        <div class="card-body">
                            <h3 class="card-title pt-3">Enterprise</h3>
                            <h2 class="card-title text-primary mb-0 pt-4">$499</h2>
                            <div class="text-muted font-weight-medium mt-2">per month</div>
                            <ul class="list-unstyled pricing-list">
                                <li>Unlimited users</li>
                                <li>Unlimited websites</li>
                                <li>Access to premium templates</li>
                                <li>24/7 support</li>
                            </ul>
                            <a href="#" class="btn btn-primary">
                                Start free trial
                            </a>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        <div class="row mt-6">
            <div class="col-md-4 mr-auto">
                <h3>Everything is covered.</h3>
                <p class="lead">
                    Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum in nisi commodo, tempus odio a,
                    vestibulum nibh.
                </p>
            </div>
            <div class="col-md-7 offset-md-1">
                <ul class="features-list">
                    <li>Weekly new templates</li>
                    <li>Access to new features</li>
                    <li>MailChimp integration</li>
                    <li>Stripe integration</li>
                    <li>100 refund guarantee</li>
                    <li>Advance SEO tools</li>
                    <li>Free unlimited support</li>
                </ul>
            </div>
        </div>
        <div class="row mt-5">
            <div class="col-md-8 col-12 divider top-divider mx-auto pt-5 text-center">
                <h3>Try Knight free for 7 days</h3>
                <p class="mb-4">Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum in nisi commodo,
                    tempus odio a, vestibulum nibh.</p>
                <button class="btn btn-primary">
                    Create your account
                </button>
            </div>
        </div>
    </div>
</section> -->

<!--faq section-->
<!-- <section class="py-7" id="faq">
    <div class="container">
        <div class="row">
            <div class="col-md-6 mx-auto text-center">
                <h2>Frequently asked questions</h2>
                <p class="text-muted lead">Answers to most common questions.</p>
            </div>
        </div>
        <div class="row mt-5">
            <div class="col-md-10 mx-auto">
                <div class="row">
                    <div class="col-md-6 mb-5">
                        <h6>Can I try it for free?</h6>
                        <p class="text-muted">Nam liber tempor cum soluta nobis eleifend option congue nihil imper per tem por legere me doming.</p>
                    </div>
                    <div class="col-md-6 mb-5">
                        <h6>Do you have hidden fees?</h6>
                        <p class="text-muted">Nam liber tempor cum soluta nobis eleifend option congue nihil imper per tem por legere me doming.</p>
                    </div>
                    <div class="col-md-6 mb-5">
                        <h6>What are the payment methods you accept?</h6>
                        <p class="text-muted">Nam liber tempor cum soluta nobis eleifend option congue nihil imper per tem por legere me doming.</p>
                    </div>
                    <div class="col-md-6 mb-5">
                        <h6>How often do you release updates?</h6>
                        <p class="text-muted">Nam liber tempor cum soluta nobis eleifend option congue nihil imper per tem por legere me doming.</p>
                    </div>
                    <div class="col-md-6 mb-5">
                        <h6>What is your refund policy?</h6>
                        <p class="text-muted">Nam liber tempor cum soluta nobis eleifend option congue nihil imper per tem por legere me doming.</p>
                    </div>
                    <div class="col-md-6 mb-5">
                        <h6>How can I contact you?</h6>
                        <p class="text-muted">Nam liber tempor cum soluta nobis eleifend option congue nihil imper per tem por legere me doming.</p>
                    </div>
                </div>
            </div>
        </div>
        <div class="row mt-4">
            <div class="col-md-6 mx-auto text-center">
                <h5 class="mb-4">Have questions?</h5>
                <a href="#" class="btn btn-primary">Contact us</a>
            </div>
        </div>
    </div>
</section> -->

<!--news section-->
<!-- <section class="py-7 bg-dark section-angle top-left bottom-left" id="blog">
    <div class="container">
        <div class="row">
            <div class="col-md-6 mx-auto text-center">
                <h2 class="heading-black">News from Knight.</h2>
                <p class="text-muted lead">What's new at Knight.</p>
            </div>
        </div>
        <div class="row mt-5">
            <div class="col-md-4">
                <div class="card">
                    <a href="#">
                        <img class="card-img-top img-raised" src="img/blog-1.jpg" alt="Blog 1">
                    </a>
                    <div class="card-body">
                        <a href="#" class="card-title mb-2"><h5>We launch new iOS & Android mobile apps</h5></a>
                        <p class="text-muted small-xl mb-2">Sep 27, 2018</p>
                        <p class="card-text">Nam liber tempor cum soluta nobis eleifend option congue nihil imper,
                            consectetur adipiscing elit. <a href="#">Learn more</a></p>
                    </div>
                </div>
            </div>
            <div class="col-md-4">
                <div class="card">
                    <a href="#">
                        <img class="card-img-top img-raised" src="img/blog-2.jpg" alt="Blog 2">
                    </a>
                    <div class="card-body">
                        <a href="#" class="card-title mb-2"><h5>New update is available for the editor</h5></a>
                        <p class="text-muted small-xl mb-2">August 16, 2018</p>
                        <p class="card-text">Nam liber tempor cum soluta nobis eleifend option congue nihil imper,
                            consectetur adipiscing elit. <a href="#">Learn more</a></p>
                    </div>
                </div>
            </div>
            <div class="col-md-4">
                <div class="card">
                    <a href="#">
                        <img class="card-img-top img-raised" src="img/blog-3.jpg" alt="Blog 3">
                    </a>
                    <div class="card-body">
                        <a href="#" class="card-title mb-2"><h5>The story of building #1 page builder</h5></a>
                        <p class="text-muted small-xl mb-2">December 2nd, 2017</p>
                        <p class="card-text">Nam liber tempor cum soluta nobis eleifend option congue nihil imper,
                            consectetur adipiscing elit. <a href="#">Learn more</a></p>
                    </div>
                </div>
            </div>
        </div>
        <div class="row mt-6">
            <div class="col-md-6 mx-auto text-center">
                <a href="#" class="btn btn-primary">View all posts</a>
            </div>
        </div>
    </div>
</section> -->

<?php include "partials/footer.php" ?>