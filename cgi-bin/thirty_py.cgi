#!/usr/bin/env python3
print()

print('''
<html>
<head>
    <title>Countdown</title>
    <script type="text/javascript">
        var count = 30;
        function countdown() {
            if (count == 0) {
                document.getElementById("countdown").innerHTML = "Blast off!";
            } else {
                document.getElementById("countdown").innerHTML = count + " seconds remaining";
                count--;
                setTimeout("countdown()", 1000);
            }
        }
    </script>
</head>
<body onload="countdown()">
    <h2>Countdown from 30 to 0</h2>
    <div id="countdown"></div>
</body>
</html>
''')
