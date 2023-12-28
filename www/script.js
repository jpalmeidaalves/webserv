console.log("hello from js");

const num = 2;

console.log("2 + 2 = ", num + num);

var testEl = document.getElementById("test");

testEl.classList.add("red");

var newEl = document.createElement("p");

newEl.innerText = "more content";

testEl.append(newEl);


const ulEl = document.getElementById('users');
const url = 'http://127.0.0.1:8084/data.json';

fetch(url)
  .then((response) => {
    return response.json();
  })
  .then((data) => {
    console.log({data});
    let users = data.users;

    ulEl.innerHTML = JSON.stringify(users, null, 4);
  })
  .catch(function(error) {
    console.log(error);
  });