#!/usr/bin/php

<!DOCTYPE html>
<html>

<head>
	<title>Server Variables</title>
	<style>
		table {
			width: 100%;
			border-collapse: collapse;
		}

		table,
		th,
		td {
			border: 1px solid black;
		}

		th,
		td {
			padding: 8px;
			text-align: left;
		}

		th {
			background-color: #f2f2f2;
		}
	</style>
</head>

<body>
	<h1>$_SERVER Variables</h1>
	<table>
		<tr>
			<th>Key</th>
			<th>Value</th>
		</tr>
		<?php
		foreach ($_SERVER as $key => $value) {
			echo "<tr>";
			echo "<td>" . htmlspecialchars((string) $key) . "</td>";  // Cast key to string to avoid errors
			if (is_array($value)) {
				$value = implode(', ', $value);  // Convert array to comma-separated string
			}
			echo "<td>" . htmlspecialchars((string) $value) . "</td>";  // Cast value to string to avoid errors
			echo "</tr>";
		}
		?>
	</table>
</body>

</html>