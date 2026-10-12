# Publishing Range FEA in the Microsoft Store

This guide covers the one-time setup of Partner Center and the GitHub
repository, and the steps repeated for every release. The package is built by the
GitHub Actions workflow [`.github/workflows/store.yml`](../.github/workflows/store.yml)
(job `microsoft-store`). It runs **only when a GitHub release is published**.

Overview of what happens on a release:

1. A release is published on GitHub.
2. `store.yml` builds the application with MSVC on a Windows runner with
   `-DRANGE_STORE_BUILD=ON`. The build has no built-in updater, and stored
   install paths are reset at every start.
3. The installed files, the generated `AppxManifest.xml` and the Store logos are
   packed into an unsigned `.msix`. The Store signs the package itself, so no
   code-signing certificate is needed.
4. You download the `.msix` from the workflow run and submit it in Partner
   Center. The upload is not automated.

---

## 1. One-time setup

### 1.1 Partner Center developer account

1. Open <https://storedeveloper.microsoft.com> and choose **Get started**. Sign
   in with the Microsoft account that will own the application.
2. Choose the account type:
   - **Company**: the publisher name is the registered company name and needs
     verification, which can take several days.
   - **Individual**: no verification.
3. Complete the registration, including any one-time fee. Payout and tax
   profiles are needed only for a paid application.

### 1.2 Reserve the application name

1. <https://partner.microsoft.com/dashboard> → **Apps and games** → **+ New
   product** → **MSIX or PWA app**.
2. Enter `Range FEA`, check availability and click **Reserve product name**.

### 1.3 Package identity → repository variables `MSIX_IDENTITY_NAME`, `MSIX_PUBLISHER`, `MSIX_PUBLISHER_DISPLAY_NAME`

1. In Partner Center open the application → **Product management** → **Product
   identity**.
2. Copy the three values to the repository variables:

| Partner Center field | Repository variable | Looks like |
|---|---|---|
| Package/Identity/Name | `MSIX_IDENTITY_NAME` | `12345RangeSoftware.RangeFEA` |
| Package/Identity/Publisher | `MSIX_PUBLISHER` | `CN=1A2B3C4D-5E6F-7A8B-9C0D-1E2F3A4B5C6D` |
| Package/Properties/PublisherDisplayName | `MSIX_PUBLISHER_DISPLAY_NAME` | `Range Software` |

The workflow falls back to `RangeSoftware.RangeFEA`, `CN=Range Software` and
`Range Software`. The first two will not match the reserved identity, and
Partner Center rejects a package whose identity differs from the reservation.

### 1.4 Store the values in GitHub

Repository → **Settings** → **Secrets and variables** → **Actions** →
**Variables** → **New repository variable**, once for each of the three
variables. Or with the GitHub CLI:

```bash
gh variable set MSIX_IDENTITY_NAME --body "12345RangeSoftware.RangeFEA"
gh variable set MSIX_PUBLISHER --body "CN=1A2B3C4D-5E6F-7A8B-9C0D-1E2F3A4B5C6D"
gh variable set MSIX_PUBLISHER_DISPLAY_NAME --body "Range Software"
```

No secrets are needed for the Microsoft Store job.

### 1.5 Privacy policy

The application uses the internet (Range Cloud, the AI assistant, optional usage
statistics), so the Store requires a privacy policy URL. Publish one, for
example on `https://www.range-software.com`, before the first submission.

---

## 2. Every release

### 2.1 Prepare the version

1. Set the release version in `src/Config.cmake` (`PROJECT_VERSION`), then
   commit and push.
2. The MSIX package version is computed as
   `<major>.<minor>.<store.yml run number>.0`. For example, release 1.3.0 built
   by run 57 becomes `1.3.57.0`. The run number takes the place of the patch
   number, so every package is higher than the one before it, as the Store
   requires. The fourth component stays `0` because the Store reserves it.

### 2.2 Publish the GitHub release

1. GitHub → **Releases** → **Draft a new release**.
2. Create a new tag, for example `v1.3.0`, on the commit to release. Fill in the
   notes.
3. Saving the draft triggers nothing. Click **Publish release**. Pre-releases
   trigger the store build too.

### 2.3 Download the package

1. Repository → **Actions** → **Store packages (App Store / Microsoft Store)** →
   the run for the release → job **Microsoft Store package** must be green.
2. At the bottom of the run page, under **Artifacts**, download
   `range-fea-microsoft-store`. It is a `.zip` containing
   `range-fea-<version>-<system>-<processor>.msix`. Unpack it.

With the GitHub CLI:

```bash
gh run list --workflow store.yml --limit 1
gh run download <run id> --name range-fea-microsoft-store
```

If the job failed in **Configure** or **Pack MSIX**, the log names the problem,
for example *makeappx.exe was not found*. Fix it and use **Re-run jobs**.

### 2.4 Test the package locally (recommended)

An unsigned MSIX cannot be installed, but the unpacked layout can be registered
in Developer Mode:

1. Windows **Settings** → **System** → **For developers** → turn on **Developer
   Mode**.
2. Build the same layout locally (see section 3), then in PowerShell:

   ```powershell
   Add-AppxPackage -Register .\build-store\msix-staging\AppxManifest.xml
   ```

3. Start **Range FEA** from the Start menu. Check that a model can be opened,
   solved and saved, and that the help and materials load. Remove the test
   installation with `Get-AppxPackage *RangeFEA* | Remove-AppxPackage`.

Optionally run the **Windows App Certification Kit**, part of the Windows SDK.
It performs the same technical checks as Store certification:

```powershell
& "C:\Program Files (x86)\Windows Kits\10\App Certification Kit\appcert.exe" test `
    -appxpackagepath .\range-fea-<version>.msix -reportoutputpath .\wack-report.xml
```

### 2.5 Create the submission in Partner Center

Partner Center → **Apps and games** → **Range FEA** → **Start submission**. For
later releases use **Update** on the existing submission.

1. **Pricing and availability**: markets, visibility (public or private
   audience) and price (free or paid).
2. **Properties**:
   - category **Productivity**, or **Education** if preferred;
   - privacy policy URL (1.5);
   - website and support contact.
   - System requirements: a 64-bit processor, plus OpenGL or Vulkan graphics
     for the 3D view.
3. **Age ratings**: fill in the IARC questionnaire. The application has no
   objectionable content but does communicate over the internet.
4. **Packages**: drag in the `.msix` from 2.3. Partner Center validates the
   identity (1.3) and the version. Device family: **Windows 10/11 Desktop**.
5. **Store listings**: the package declares the languages `en-us` and `sk-sk`,
   so add a listing for **both** English and Slovak. Each needs a description,
   at least one screenshot (1366×768 or larger, PNG), and optionally a short
   description and keywords.
6. **Submission options**:
   - Publishing hold: publish as soon as certified, or manually.
   - **Restricted capabilities**: the package declares `runFullTrust`. Explain
     that Range FEA is a packaged Win32 desktop application which launches its
     bundled solver (`fea-solver.exe`) and OpenSSL tool as separate processes.
   - **Notes for certification**: describe how to try the application, for
     example by opening a sample model from the data folder and starting the
     solver. Mention whether a Range Cloud account is needed for any feature.
7. **Submit to the Store**.

### 2.6 Certification and release

1. Certification takes up to three business days. Progress shows on the
   submission page.
2. If the submission fails, the report lists the failed checks. Fix them,
   publish a new GitHub release (or re-run the workflow, which raises the
   version automatically), and upload the new `.msix` into the same submission.
3. After certification the application goes live immediately or waits for
   **Publish now**, depending on the publishing hold chosen in 2.5. Updates reach
   existing users through the Store automatically.

---

## 3. Building the MSIX by hand

From a **Developer PowerShell for VS 2022**, with Ninja, OpenSSL in
`C:\OpenSSL`, Qt 6.12 for MSVC 2022 in `CMAKE_PREFIX_PATH`, and the Windows SDK
installed (it provides `makeappx.exe`):

```powershell
cmake -S src -B build-store -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DRANGE_STORE_BUILD=ON `
  -DRANGE_BUILD_NUMBER=<next unused build number> `
  -DOPENSSL_ROOT_DIR="C:\OpenSSL" `
  -DMSIX_IDENTITY_NAME="12345RangeSoftware.RangeFEA" `
  -DMSIX_PUBLISHER="CN=1A2B3C4D-5E6F-7A8B-9C0D-1E2F3A4B5C6D" `
  -DMSIX_PUBLISHER_DISPLAY_NAME="Range Software"
cmake --build build-store --parallel
cmake --build build-store --target msix
```

The package is written to `build-store\<package name>.msix`. The unpacked
layout used in 2.4 is `build-store\msix-staging`. If CMake reports that
`makeappx.exe` was not found, add the SDK tools folder to `PATH`, for example
`C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64`, and configure
again.

The build number must be higher than the build number of every package already
uploaded, so pick one above the latest `store.yml` run number.
