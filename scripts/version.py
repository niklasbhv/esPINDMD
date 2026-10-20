# Injects the firmware version and the PlatformIO environment into the build.
#
# The version is taken from the ESPINDMD_VERSION environment variable (set by
# the release workflow) and falls back to `git describe` for local builds.
# The environment name is used by the OTA update to pick the matching binary.
#
# Only applied to the project sources, so a new version does not trigger a
# rebuild of the framework and libraries.

import os
import subprocess

Import("env", "projenv")


def get_version():
    version = os.environ.get("ESPINDMD_VERSION")
    if not version:
        try:
            version = subprocess.check_output(
                ["git", "describe", "--tags", "--always", "--dirty"],
                cwd=env["PROJECT_DIR"],
                stderr=subprocess.DEVNULL,
            ).decode().strip()
        except (OSError, subprocess.CalledProcessError):
            version = "0.0.0-dev"
    return version[1:] if version.startswith("v") else version


version = get_version()
print("esPINDMD version: " + version)
projenv.Append(
    CPPDEFINES=[
        ("ESPINDMD_VERSION", env.StringifyMacro(version)),
        ("ESPINDMD_ENV", env.StringifyMacro(env["PIOENV"])),
    ]
)
