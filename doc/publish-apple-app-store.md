# Publishing Range FEA in the Apple Mac App Store

This guide covers the one-time setup of the Apple accounts and the GitHub
repository, and the steps repeated for every release. The package itself is
built and uploaded by the GitHub Actions workflow
[`.github/workflows/store.yml`](../.github/workflows/store.yml) (job
`mac-app-store`). It runs **only when a GitHub release is published**.

Overview of what happens on a release:

1. A release is published on GitHub.
2. `store.yml` builds the application on a macOS runner with
   `-DRANGE_STORE_BUILD=ON`. The bundle is sandboxed, has no built-in updater,
   and is signed with the App Store certificates.
3. The bundle is wrapped in a signed installer package (`.pkg`) and uploaded to
   App Store Connect.
4. In App Store Connect you test the build through TestFlight and submit it for
   review. You also decide when it goes on sale.

---

## 1. One-time setup

### 1.1 Apple Developer Program membership

1. Enrol at <https://developer.apple.com/programs/enroll/> with the Apple Account
   that will own the application. A company enrolment needs a D-U-N-S number.
   The fee is yearly.
2. Wait for the enrolment to be approved. Everything below needs an active
   membership.

### 1.2 Team ID → repository variable `MACOS_TEAM_ID`

1. Open <https://developer.apple.com/account> and go to **Membership details**.
2. Copy the **Team ID**, a 10-character value such as `ABCDE12345`.

Without it the `mac-app-store` CMake target is not created and the workflow fails.

### 1.3 Bundle identifier → repository variable `MACOS_BUNDLE_IDENTIFIER`

The workflow uses `com.range-software.fea` unless the variable is set. Register
the identifier once:

1. In <https://developer.apple.com/account/resources/identifiers/list> click **+**.
2. Choose **App IDs** → **App**, then **Continue**.
3. Description: `Range FEA`. Bundle ID: **Explicit**, `com.range-software.fea`.
4. Leave all capabilities unticked. The application needs none beyond the App
   Sandbox, which comes from the entitlements.
5. Click **Continue**, then **Register**.

If you register a different identifier, set the repository variable
`MACOS_BUNDLE_IDENTIFIER` to it. The bundle identifier can never change once a
build has been uploaded.

### 1.4 Signing certificates → secrets `MAC_APP_STORE_CERT_P12`, `MAC_APP_STORE_CERT_P12_PASSWORD`, `MACOS_APP_STORE_APP_CERT`, `MACOS_APP_STORE_INSTALLER_CERT`

Two certificates are needed. Both are created on a Mac.

| Certificate type in the portal | Signs | Identity name in the keychain |
|---|---|---|
| **Mac App Distribution** | the application bundle | `3rd Party Mac Developer Application: <Team name> (<Team ID>)` |
| **Mac Installer Distribution** | the `.pkg` installer | `3rd Party Mac Developer Installer: <Team name> (<Team ID>)` |

Create them:

1. On a Mac open **Keychain Access** → **Keychain Access** menu → **Certificate
   Assistant** → **Request a Certificate From a Certificate Authority**.
   - Enter your e-mail and a common name, and leave the CA e-mail empty.
   - Select **Saved to disk**. This produces a `.certSigningRequest` file.
2. In <https://developer.apple.com/account/resources/certificates/list> click **+**.
   - Select **Mac App Distribution**, then **Continue**.
   - Upload the request file and download the `.cer`.
3. Repeat step 2 for **Mac Installer Distribution**. The same request file can be
   reused.
4. Double-click both `.cer` files to import them into the **login** keychain.

Export both identities into **one** `.p12` file:

1. In Keychain Access → **My Certificates**, select both `3rd Party Mac Developer
   …` entries. Each must have a private key underneath it.
2. Right-click → **Export 2 items…** → format **Personal Information Exchange
   (.p12)**, for example `appstore.p12`.
3. Choose a strong password. It becomes the secret `MAC_APP_STORE_CERT_P12_PASSWORD`.
4. Encode the file for the secret `MAC_APP_STORE_CERT_P12`:

   ```bash
   base64 -i appstore.p12 -o appstore.p12.base64.txt
   ```

Read the exact identity names for the two remaining secrets:

```bash
security find-identity -v
```

- `MACOS_APP_STORE_APP_CERT` gets the full name in quotes, for example
  `3rd Party Mac Developer Application: Range Software (ABCDE12345)`.
- `MACOS_APP_STORE_INSTALLER_CERT` gets
  `3rd Party Mac Developer Installer: Range Software (ABCDE12345)`.

If `find-identity` does not list the installer certificate, copy its **Common
Name** from Keychain Access instead. It is the same string.

Keep `appstore.p12` and its password in a password manager, then delete the
local copies of the `.p12` and the `.txt`.

### 1.5 Provisioning profile → secret `MAC_PROVISION_PROFILE`

1. In <https://developer.apple.com/account/resources/profiles/list> click **+**.
2. Under **Distribution** select **Mac App Store Connect**, then **Continue**.
3. Profile type **Mac**. App ID: the one registered in 1.3.
4. Certificate: the **Mac App Distribution** certificate from 1.4.
5. Name it `Range FEA App Store`, then **Generate** → **Download** to get a
   `.provisionprofile` file.
6. Encode it:

   ```bash
   base64 -i Range_FEA_App_Store.provisionprofile -o profile.base64.txt
   ```

The profile expires together with the certificate, after one year. Renew both
and update the secrets when that happens.

### 1.6 App Store Connect API key → secrets `AC_API_KEY_ID`, `AC_API_ISSUER_ID`, `AC_API_KEY_P8`

The release workflow `ci.yml` already uses these secrets for notarization. The
same key also works for the upload if its role is **App Manager** or **Admin**.
If the existing key has the **Developer** role, create a new one:

1. <https://appstoreconnect.apple.com> → **Users and Access** → **Integrations** →
   **App Store Connect API** → **Team Keys** → **+**.
2. Name `GitHub Actions`, access **App Manager**, then **Generate**.
3. Copy the **Issuer ID** shown above the table → secret `AC_API_ISSUER_ID`.
4. Copy the **Key ID** of the new key → secret `AC_API_KEY_ID`.
5. **Download** the `AuthKey_<KeyID>.p8`. It can be downloaded only once. Encode it:

   ```bash
   base64 -i AuthKey_XXXXXXXXXX.p8 -o authkey.base64.txt
   ```

   The encoded text becomes the secret `AC_API_KEY_P8`.

### 1.7 Keychain password → secret `MAC_KEYCHAIN_PASSWORD`

This is the password of the temporary keychain created on the runner. Any random
string works. The secret already exists if `ci.yml` signs macOS builds; otherwise
generate one:

```bash
openssl rand -base64 24
```

### 1.8 Store the values in GitHub

Repository → **Settings** → **Secrets and variables** → **Actions**.

| Name | Kind | Value from |
|---|---|---|
| `MACOS_TEAM_ID` | Variable | 1.2 |
| `MACOS_BUNDLE_IDENTIFIER` | Variable, optional | 1.3, defaults to `com.range-software.fea` |
| `MAC_APP_STORE_CERT_P12` | Secret | 1.4, contents of `appstore.p12.base64.txt` |
| `MAC_APP_STORE_CERT_P12_PASSWORD` | Secret | 1.4 |
| `MACOS_APP_STORE_APP_CERT` | Secret | 1.4 |
| `MACOS_APP_STORE_INSTALLER_CERT` | Secret | 1.4 |
| `MAC_PROVISION_PROFILE` | Secret | 1.5, contents of `profile.base64.txt` |
| `AC_API_KEY_ID` | Secret | 1.6 |
| `AC_API_ISSUER_ID` | Secret | 1.6 |
| `AC_API_KEY_P8` | Secret | 1.6, contents of `authkey.base64.txt` |
| `MAC_KEYCHAIN_PASSWORD` | Secret | 1.7 |

With the GitHub CLI, the same can be done from the folder holding the files:

```bash
gh variable set MACOS_TEAM_ID --body "ABCDE12345"
gh secret set MAC_APP_STORE_CERT_P12 < appstore.p12.base64.txt
gh secret set MAC_APP_STORE_CERT_P12_PASSWORD
gh secret set MACOS_APP_STORE_APP_CERT --body "3rd Party Mac Developer Application: Range Software (ABCDE12345)"
gh secret set MACOS_APP_STORE_INSTALLER_CERT --body "3rd Party Mac Developer Installer: Range Software (ABCDE12345)"
gh secret set MAC_PROVISION_PROFILE < profile.base64.txt
gh secret set AC_API_KEY_P8 < authkey.base64.txt
```

### 1.9 Create the application record in App Store Connect

The upload fails until the record exists.

1. <https://appstoreconnect.apple.com> → **Apps** → **+** → **New App**.
2. Platforms **macOS**. Name `Range FEA`, which must be unique in the store.
   Primary language **English (U.S.)**.
3. Bundle ID: the one from 1.3. SKU: any internal identifier, for example
   `range-fea`. User access **Full Access**.
4. Click **Create**.

---

## 2. Every release

### 2.1 Prepare the version

1. Set the release version in `src/Config.cmake` (`PROJECT_VERSION`). It becomes
   `CFBundleShortVersionString`, the version customers see. App Store Connect
   accepts each version string for one approved release only, so every
   submission to review needs a new version.
2. Update the release notes, commit and push.

The build number (`CFBundleVersion`) is set automatically to the run number of
`store.yml`. It grows with every run, so re-running a failed release needs no
manual change.

### 2.2 Publish the GitHub release

1. GitHub → **Releases** → **Draft a new release**.
2. Create a new tag, for example `v1.3.0`, on the commit to release. Fill in the
   notes.
3. Saving the draft triggers nothing. Click **Publish release**. Pre-releases
   trigger the store build too.

Publishing starts two workflows: `CI (Linux/macOS/Windows)` for the GitHub and
Range Cloud packages, and `Store packages (App Store / Microsoft Store)`.

### 2.3 Follow the store workflow

Repository → **Actions** → **Store packages (App Store / Microsoft Store)** →
the run for the release → job **Apple App Store package**.

- **Configure** fails with *MACOS_TEAM_ID is empty*: set the variable from 1.2.
- **Import App Store certificates** lists no identities: the `.p12` or its
  password is wrong (1.4).
- **Verify package** fails: the bundle lacks the provisioning profile or the
  application identifier entitlement. Check `MAC_PROVISION_PROFILE` and
  `MACOS_TEAM_ID`.
- **Upload to App Store Connect** fails with an authentication error: the API
  key role is too low or a key secret is wrong (1.6). A rejection message that
  names an `ITMS-` code comes from Apple's validation. Fix the cause and re-run
  the job.

The signed `.pkg` is also kept as the artifact `range-fea-mac-app-store` on the
run page.

To re-run after fixing a secret, use **Re-run jobs** on the same run. A failed
upload does not use up the build number.

### 2.4 Test with TestFlight

1. App Store Connect e-mails you when the build has been processed, usually
   within 15–60 minutes. It appears under **TestFlight**.
2. Under **Export Compliance** nothing has to be answered. The bundle declares
   `ITSAppUsesNonExemptEncryption = NO`.
3. Add yourself to an internal testing group, install **TestFlight** from the Mac
   App Store, and install the build.

A bundle signed for the App Store does not launch when started directly.
TestFlight is the way to run it. Check at least:

- opening, saving and solving a model stored **outside** the app container (for
  example on the Desktop), including a transient run that writes several result
  records;
- the help browser, the materials database and Range Cloud login.

### 2.5 Fill in the store listing (first release, then as needed)

In App Store Connect → the app:

- **App Information**: category (Productivity or Education), content rights,
  age rating questionnaire.
- **Pricing and Availability**: price and countries.
- **App Privacy**: privacy policy URL (required) and the data-collection
  questionnaire. Range Cloud, the AI assistant and the optional usage
  statistics send data over the network; describe them.
- **The version page** (`macOS App` → version):
  - screenshots in 16:10, one of 1280×800, 1440×900, 2560×1600 or 2880×1800,
    at least one and up to ten;
  - description, keywords, support URL, marketing URL, copyright;
  - **App Review Information**: contact details, and a demo account if a
    reviewer needs Range Cloud access to see the features. Note in the review
    notes that the solver runs as a bundled helper executable.

### 2.6 Submit for review and release

1. On the version page, under **Build**, click **+** and select the build tested
   in 2.4.
2. Choose the release option: **Manually release this version** or
   **Automatically release this version** after approval.
3. **Add for Review** → **Submit for Review**.
4. Review usually takes one to three days. If the build is rejected, the reason
   appears in the **Resolution Center**. Fix it, publish a new GitHub release
   (or re-run the workflow), select the new build and submit again.
5. After approval, if manual release was chosen, click **Release This Version**.

---

## 3. Building the store package by hand

To reproduce the workflow on a Mac with the certificates in the login keychain
(Homebrew `ninja`, `libomp`, `openssl@3`, and Qt 6.12 in `CMAKE_PREFIX_PATH`):

```bash
cmake -S src -B build-store -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DRANGE_STORE_BUILD=ON \
  -DRANGE_BUILD_NUMBER=<next unused build number> \
  -DOpenMP_INCLUDES="$(brew --prefix libomp)/include" \
  -DOpenMP_LIBRARY="$(brew --prefix libomp)/lib/libomp.dylib" \
  -DOPENSSL_ROOT_DIR="$(brew --prefix openssl@3)" \
  -DMACOS_TEAM_ID=ABCDE12345 \
  -DMACOS_APP_STORE_APP_CERT="3rd Party Mac Developer Application: Range Software (ABCDE12345)" \
  -DMACOS_APP_STORE_INSTALLER_CERT="3rd Party Mac Developer Installer: Range Software (ABCDE12345)" \
  -DMACOS_PROVISION_PROFILE="$HOME/Downloads/Range_FEA_App_Store.provisionprofile"
cmake --build build-store --parallel
cmake --build build-store --target mac-app-store
```

The `.pkg` lands in `build-store/`. Upload it with **Transporter** from the Mac
App Store, or:

```bash
xcrun altool --upload-app --type macos --file build-store/<package>.pkg \
  --apiKey <Key ID> --apiIssuer <Issuer ID>
```

`altool` looks for `AuthKey_<Key ID>.p8` in `~/.private_keys`. The build number
must be higher than any build already uploaded, so pick one above the latest
`store.yml` run number.
