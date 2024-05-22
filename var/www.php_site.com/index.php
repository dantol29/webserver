#!/usr/bin/php

<?php
session_start();

$databaseFile = 'database.txt';

function saveUser($username, $password) {
    global $databaseFile;
    $userData = $username . ',' . $password . PHP_EOL;
    file_put_contents($databaseFile, $userData, FILE_APPEND);
}

function getUser($username) {
    global $databaseFile;
    if (!file_exists($databaseFile)) {
        return null;
    }
    $users = file($databaseFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    foreach ($users as $user) {
        list($storedUsername, $storedPassword) = explode(',', $user);
        if ($storedUsername === $username) {
            return $storedPassword;
        }
    }
    return null;
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (isset($_POST['signup'])) {
        $username = trim($_POST['username']);
        $password = trim($_POST['password']);

        if ($username && $password) {
            if (getUser($username)) {
                $error = 'Username already exists';
            } else {
                saveUser($username, $password);
                $message = 'Signup successful!';
            }
        } else {
            $error = 'Please enter both username and password';
        }
    } elseif (isset($_POST['signin'])) {
        $username = trim($_POST['username']);
        $password = trim($_POST['password']);

        if ($username && $password) {
            $storedPassword = getUser($username);
            if ($storedPassword && $storedPassword === $password) {
                $_SESSION['username'] = $username;
                header('Location: welcome.php');
                exit;
            } else {
                $error = 'Invalid username or password';
            }
        } else {
            $error = 'Please enter both username and password';
        }
    }
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>Sign Up and Sign In</title>
</head>
<body>
    <h2>Sign Up</h2>
    <?php if (isset($error)) echo '<p style="color:red;">' . $error . '</p>'; ?>
    <?php if (isset($message)) echo '<p style="color:green;">' . $message . '</p>'; ?>
    <form method="POST">
        <label for="signup-username">Username:</label>
        <input type="text" id="signup-username" name="username" required>
        <br>
        <label for="signup-password">Password:</label>
        <input type="password" id="signup-password" name="password" required>
        <br>
        <button type="submit" name="signup">Sign Up</button>
    </form>

    <h2>Sign In</h2>
    <form method="POST">
        <label for="signin-username">Username:</label>
        <input type="text" id="signin-username" name="username" required>
        <br>
        <label for="signin-password">Password:</label>
        <input type="password" id="signin-password" name="password" required>
        <br>
        <button type="submit" name="signin">Sign In</button>
    </form>
</body>
</html>
