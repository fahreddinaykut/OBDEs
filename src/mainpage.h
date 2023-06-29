const char MAIN_PAGE[] = R"=====(
<html>

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css"
        integrity="sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u" crossorigin="anonymous">
    <style>
        hr {
            height: 2px;
            border-width: 0px;
            background-color: rgb(97, 155, 158);
            color: aqua;
            
        }

        body {
            font-family: Arial, Helvetica, sans-serif;
            background-color: #cecece;
            padding-bottom: 10px;
            padding-top: 10px;
        }

        .container {
            margin: auto;
            width: 98%;
            padding: 10px;
          
            background-color: rgb(34, 34, 34);
            border-radius: 6px;
            text-align: center;
        }

        button {
            background-color: rgb(97, 155, 158);
            width: 30%;
            color: rgb(255, 255, 255);
            padding: 10px;
            margin-top: 20px;
            margin-bottom: 10px;
            margin-right: auto;
            margin-left: auto;
            border: 1px solid rgb(119, 119, 119);
            cursor: pointer;
            border-radius: 1px;
            font-size: small;
            font-family: 'Trebuchet MS', 'Lucida Sans Unicode', 'Lucida Grande', 'Lucida Sans', Arial, sans-serif;

        }

        .center {
            margin-left: auto;
            margin-right: auto;
        }

        .textboxStyle {
            width: 100%;
            height: 450px;
            line-height: 1.4em;
            overflow: auto;
            padding: 5px;
            color: rgb(255, 255, 255);
            transition: top 0.3s;
            font-family: 'Trebuchet MS', 'Lucida Sans Unicode', 'Lucida Grande', 'Lucida Sans', Arial, sans-serif;

        }

        label {
            color: rgb(194, 194, 194);
            font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
        }

        h1 {
            color: rgb(255, 255, 255);
        }
    </style>

    <title>OBD ES</title>
</head>

<body>

    <div class="container">
        <h1 class="text-center" style="color: rgb(97, 155, 158);">RIDE OBD2 Test Interface</h1>
        <label id="ipLabel">IP:</label>
        <hr>
        <div>
            <label id="speedLabel">SPEED:-1</label>
            <label id="rpmLabel">RPM:-1</label>
            <label id="tempLabel">TEMP:-1</label>
        </div>
        <hr>
        <div class="textboxStyle" id="textbox1">
        </div>
        <hr>
        <label> <input onclick="myFunction()" id="myCheck" type="checkbox" checked> AutoScroll</label>
        <button onclick="clearTerminal()" type="button">Clear Terminal</button>
        <button onclick="savePID()" type="button">Send test message!</button>

    </div>

    </div>


    <script>
        const d = new Date();
        let ms = d.getMilliseconds();
        var lastreceiveMessage = 0;
        var autoscrool = true;
        function savePID() {

            var testmessage = "testing send data from page";
            var testmessage2 = "message2";
            // wait for answer callback
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {

                }
            };
            // create querrystring
            var sendStr = "savePID?p=" + testmessage + "&i=" + testmessage2;
            // console.log(sendStr);

            // send querrystring using xmlhttprequest
            xhttp.open("GET", sendStr, true);
            xhttp.send();
        }


        setInterval(function () {
            // Call a function repetatively with 2 Second interval
            getData();
            getCarData();
            getIPAddress();
        }, 100); //2000mSeconds update rate
        function getData() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    const myArray = this.responseText.split("|");
                    if (parseInt(myArray[1]) != lastreceiveMessage) {
                        document.getElementById("textbox1").innerHTML +=
                            myArray[1] + ": " + myArray[0] + "<br>";
                        lastreceiveMessage = parseInt(myArray[1]);
                    }
                    console.log(this.responseText);
                }
                if (autoscrool) {
                    var objDiv = document.getElementById("textbox1");
                    objDiv.scrollTop = objDiv.scrollHeight;
                }
            };
            xhttp.open("GET", "message", true);
            console.log(xhttp.status);
            xhttp.send();


        }
        function getCarData() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    const myArray = this.responseText.split("|");
                    document.getElementById("speedLabel").innerHTML = "SPEED:" + myArray[0];
                    document.getElementById("rpmLabel").innerHTML = "RPM:" + myArray[1];
                    document.getElementById("tempLabel").innerHTML = "TEMP:" + myArray[2];
                    // console.log(this.responseText);
                }
            };
            xhttp.open("GET", "carData", true);
            console.log(xhttp.status);
            xhttp.send();
        }
        function getIPAddress() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    const myArray = this.responseText.split("|");
                    document.getElementById("ipLabel").innerHTML = "IP:" + myArray[0];
                    // console.log(this.responseText);
                }
            };
            xhttp.open("GET", "ipAddress", true);
            console.log(xhttp.status);
            xhttp.send();
        }
        function clearTerminal() {
            document.getElementById("textbox1").innerHTML = "";
        }
        function myFunction() {
            // Get the checkbox
            var checkBox = document.getElementById("myCheck");

            // If the checkbox is checked, display the output text
            if (checkBox.checked == true) {
                autoscrool = true
            } else {
                autoscrool = false;
            }
        }

    </script>
</body>

</html>
</html>
)=====";
const char PAGE_404[] = R"====(
<html>
    <head>
        <title>404</title>
    </head>
    <body>
        <center><h1>ERROR CODE 404</h1></center>
        <center><h1>REQUESTED PAGE NOT FOUND</h1></center>
    </body>
</html>
)====";
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Captive Portal Demo</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h3>Captive Portal Demo</h3>
  <br><br>
  <form action="/get">
    <br>
    Name: <input type="text" name="name">
    <br>
    ESP32 Proficiency: 
    <select name = "proficiency">
      <option value=Beginner>Beginner</option>
      <option value=Advanced>Advanced</option>
      <option value=Pro>Pro</option>
    </select>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";