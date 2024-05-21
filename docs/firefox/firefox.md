# Firefox Shut up Configuration

To apply custom preferences in Firefox and minimize unwanted background requests, follow these steps:

## Step 1: Create the Configuration File

1. Create a JavaScript file named `firefox.cfg`.
2. Add your desired configuration settings to this file. (See the provided `firefox.cfg` file content for reference.)

## Step 2: Create a Local Configuration File

1. Create another JavaScript file named `local-settings.js`.
2. Add the following content to `local-settings.js` to specify the configuration file:

   ```javascript
   // local-settings.js
   pref("general.config.filename", "firefox.cfg");
   pref("general.config.obscure_value", 0); // Use 0 to disable byte-shifting
   ```

## Step 3: Place the Files in the Correct Directories

1. Locate the root directory of your Firefox installation. On Windows, this is typically `C:\Program Files\Mozilla Firefox`. On macOS and Linux, it may vary.
2. Place `firefox.cfg` in the root directory of the Firefox installation.
3. Place `local-settings.js` in the `defaults/pref` directory within the Firefox installation directory.

   - **Windows Example:**
     - `firefox.cfg` -> `C:\Program Files\Mozilla Firefox\firefox.cfg`
     - `local-settings.js` -> `C:\Program Files\Mozilla Firefox\defaults\pref\local-settings.js`

## Step 4: Restart Firefox

After placing these files in the correct directories, restart Firefox. It should now load the preferences specified in your `firefox.cfg` file.
