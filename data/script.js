// Handle Top Navigation Bar
function handleNavBar() {
  var x = document.getElementById("myTopnav");
  if (x.className === "topnav") {
    x.className += " responsive";
  } 
  else {
    x.className = "topnav";
  }
}
function toggleCheckbox (element) {
  if (element.checked) {
    document.getElementById("MQTTFingerOn").style.display = "block";
  }
  else {
    document.getElementById("MQTTFingerOn").style.display = "none"; 
  }
}