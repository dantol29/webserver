#!/usr/bin/perl
print "Content-type: text/html\n\n";
print "<html><body>";
print "<h1>Welcome to the Perl Website</h1>";
print "Today's date is: " . localtime() . "<br/>";
print <<EOF;
<form action="/perl_site/cgi-bin/calculate.cgi" method="post">
  Number 1: <input type="text" name="num1"><br>
  Number 2: <input type="text" name="num2"><br>
  Operation: 
  <select name="operation">
    <option value="add">Add</option>
    <option value="subtract">Subtract</option>
  </select>
  <input type="submit" value="Calculate">
</form>
<p>Fun story about Perl <a href="/perl_site/fun_fact/php_not_perl.html">Check it out here.</a></p>
</body></html>
EOF
