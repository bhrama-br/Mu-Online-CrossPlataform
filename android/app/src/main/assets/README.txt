You can place custom packaged MU client data under:

Data/

Example:
android/app/src/main/assets/Data/Configs/Configs.xtm

On the first Android launch, the bootstrap copies this packaged Data tree
to the app's internal filesystem and uses that extracted copy as the game
data root.

The Android build also auto-syncs the original client Data from:

../../../Client/Data

into generated APK assets by default, so manual copying here is optional.

Optional packaged configs:

- android/app/src/main/assets/config.ini
- android/app/src/main/assets/client_runtime.ini

If those files are not present manually, the Android build also tries to copy:

- ../../../Client/config.ini
- ../../../Client/client_runtime.ini

Useful Android debug keys for client_runtime.ini under [Runtime]:

- ConnectServerHost=ip-or-hostname
- ConnectServerPort=44405
- GameServerHost=ip-or-hostname
- GameServerPort=55901

These runtime overrides let you test another ConnectServer on Android
without editing Configs.xtm, and they also let you replace a private
GameServer address returned by the server list during external testing.
