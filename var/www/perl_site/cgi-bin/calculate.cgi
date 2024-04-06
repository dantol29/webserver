#!/usr/bin/perl
use strict;
use warnings;
use CGI;

my $q = CGI->new;
print $q->header('text/html');

# Retrieve data from POST request
my $num1 = $q->param('num1');
my $num2 = $q->param('num2');
my $operation = $q->param('operation');

# Perform calculation
my $result;
if ($operation eq 'add') {
    $result = $num1 + $num2;
} elsif ($operation eq 'subtract') {
    $result = $num1 - $num2;
} else {
    # Default message if operation is not supported
    print "<html><body>Unsupported operation.</body></html>";
    exit;
}

# Display result
print <<EOF;
<html>
<body>
    <h1>Calculation Result</h1>
    <p>The result is: $result</p>
    <a href="/perl_site/index.cgi">Back to Home</a>
</body>
</html>
EOF
