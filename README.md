# TEAM CHRONOS — Qt Widgets desktop app

## Files
- `TEAM_CHRONOS.cpp`: your original backend source, kept unchanged.
- `TEAM_CHRONOS_Qt.cpp`: Qt Widgets desktop interface.
- `users.txt`: account database copied from your uploaded `users(1).txt`.
- `inventory.txt`: watch catalog database copied from your uploaded file.
- `CMakeLists.txt`: Qt 6 / CMake build configuration.

## Build on macOS

Open Terminal and change to this folder:

```bash
cd /path/to/team_chronos_qt
cmake -S . -B build
cmake --build build
./build/TEAM_CHRONOS
```

If CMake cannot find Qt, configure with your Qt installation prefix, for example:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
```

If Qt was installed using the Qt online installer, use the path to the Qt version
folder that contains `lib/cmake/Qt6`.

## Important data-file note
The application reads and writes `users.txt` and `inventory.txt` relative to its
current working directory. Run it from this project folder when using the
commands above. Keep backups of both files before testing admin edits or checkout.

## Existing demo accounts
The uploaded `users.txt` defines the accounts. The Admin account shown in that
file is `Chris`; use the password configured in your own file. Do not distribute
the sample credentials in a production deployment.

## Current payment behavior
The original `Payment::process()` is a simulation that always reports success.
No real KPay, AYA Pay, CB Pay, or Wave Pay transaction is made. A payment-gateway
integration and secure password storage are separate production tasks.
