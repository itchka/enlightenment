# -*- python -*-
# ex: set syntax=python:

import engine
from engine import Package

# add description of packages and dependencies here
# Packages() is defined in engine.py in this same folder.
#
# Package parameters:
#    name: string name of the package
#    dependencies: hash of list of strings, hash key is the platform
#       or "common" for the always-used dependencies
#    configure_flags: hash of list of strings, hash key is the platform
#       or "common" for the always-used configure options
#    env: hash of hash of strings, parent hash key is the platform or
#       "common" for the always-used environment variables.
#    doc_target: make target to build docs
#    test_target: make target to build and run tests
#    exclusive_platforms: list of platforms to limit this package,
#        no list means it will be used in all platforms
#

engine.platforms_set(["win", "linux"])
engine.env_default_set({
    "common": {
        "CFLAGS": "${CFLAGS} -O2 -Wall -Wextra -Wshadow",
        "CXXFLAGS": "${CXXFLAGS} -O2 -Wall -Wextra -Wshadow",
        "LDFLAGS": "${LDFLAGS}",
        },
    "linux": {
        "CFLAGS": "-fvisibility=hidden -fdata-sections -ffunction-sections",
        "CXXFLAGS": "-fvisibility=hidden -fdata-sections -ffunction-sections",
        "LDFLAGS": "-fvisibility=hidden -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--as-needed"
        },
    })


Package(
    name="efl",
    test_target="check",
    doc_target="doc",
    configure_flags={
        "common": [
            "--enable-doc",
            "--with-tests=coverage"
            ],
        },
    )
# Do we need any of this old ecore configure flags for efl now?
#    configure_flags={
#        "common": [
#            "--enable-thread-safety",
#            "--enable-ecore-evas",
#            "--enable-ecore-evas-software-buffer",
#            "--enable-ecore-imf",
#            "--enable-ecore-imf-evas",
#            "--enable-ecore-input",
#            "--enable-ecore-input-evas",
#            ],
#        "linux": [
#            "--enable-ecore-evas-software-x11",
#            "--enable-ecore-evas-opengl-x11",
#            ],
#        },
#    )

Package(
    name="edje",
    test_target="coverage",
    doc_target="doc",
    dependencies={
        "common": ["efl"],
        },
    configure_flags={
        "common": [
            "--enable-tests",
            "--enable-doc",
            "--enable-coverage",
            ],
        },
    )

Package(
    name="efreet",
    test_target="coverage",
    doc_target="doc",
    dependencies={
        "common": ["efl", "edbus"],
        },
    configure_flags={
        "common": [
            "--enable-tests",
            "--enable-doc",
            "--enable-coverage",
            ],
        },
    )

Package(
    name="eeze",
    test_target="check",
    doc_target="doc",
    exclusive_platforms=["linux"],
    dependencies={
        "common": ["efl"],
        },
    configure_flags={
        "common": [
            "--enable-doc",
            "--enable-tests",
            "--with-mount",
            "--with-umount",
            "--with-eject",
            ],
        },
    )

Package(
    name="emotion",
    doc_target="doc",
    dependencies={
        "common": ["efl", "eeze", "edje"],
        },
    )

Package(
    name="efx",
    doc_target="doc",
    dependencies={
        "common": ["efl"],
        },
    )

Package(
    name="e_dbus",
    doc_target="doc",
    exclusive_platforms=["linux"],
    dependencies={
        "common": ["efl"],
        },
    configure_flags={
        "common": [
            "--enable-doc",
            "--enable-ebluez",
            "--enable-econnman0_7x",
            "--enable-enotify",
            "--enable-eofono",
            "--enable-eukit",
            ],
        },
    )

Package(
    name="edbus",
    doc_target="doc",
    exclusive_platforms=["linux"],
    dependencies={
        "common": ["efl"],
        },
    )

Package(
    name="elementary",
    dependencies={
        "common": ["efl", "edje", "e_dbus", "efreet", "emotion"],
        },
    )

Package(
    name="clouseau",
    dependencies={
        "common": ["efl", "edje", "elementary"],
        },
    )

Package(
    name="e",
    doc_target="doc",
    exclusive_platforms=["linux"],
    dependencies={
        "common": [
            "efl",
            "edje",
            "efreet",
            "eeze",
            "e_dbus",
            "elementary",
            ],
        },
    configure_flags={
        "common": [
            "--enable-doc",
            # no autodetect, force these:
            "--enable-pam",
            "--enable-device-udev",
            "--enable-enotify",
            "--enable-enotify",
            "--enable-ibar",
            "--enable-dropshadow",
            "--enable-clock",
            "--enable-pager",
            "--enable-battery",
            "--enable-temperature",
            "--enable-notification",
            "--enable-cpufreq",
            "--enable-ibox",
            "--enable-start",
            "--enable-winlist",
            "--enable-fileman",
            "--enable-fileman-opinfo",
            "--enable-wizard",
            "--enable-conf",
            "--enable-conf-wallpaper2",
            "--enable-conf-theme",
            "--enable-conf-intl",
            "--enable-msgbus",
            "--enable-conf-applications",
            "--enable-conf-display",
            "--enable-conf-shelves",
            "--enable-conf-keybindings",
            "--enable-conf-edgebindings",
            "--enable-conf-window-remembers",
            "--enable-conf-window-manipulation",
            "--enable-conf-menus",
            "--enable-conf-dialogs",
            "--enable-conf-performance",
            "--enable-conf-paths",
            "--enable-conf-interaction",
            "--enable-gadman",
            "--enable-mixer",
            "--enable-connman",
            "--enable-illume2",
            "--enable-syscon",
            "--enable-everything",
            "--enable-systray",
            "--enable-comp",
            "--enable-shot",
            "--enable-backlight",
            "--enable-tasks",
            "--enable-conf-randr",
            ],
        },
    )

Package(
    name="terminology",
    dependencies={
        "common": ["efl", "edje", "emotion", "efreet", "elementary"],
        },
    )

Package(
    name="python-evas",
    srcdir="BINDINGS/python/python-evas",
    dependencies={"common": ["efl"]},
    # generated code sucks, no way to change it:
    env={"common": {"CFLAGS": "-Wno-unused-parameter -Wno-unused-but-set-variable -Wno-shadow -Wno-strict-aliasing"}},
    )

Package(
    name="python-ecore",
    srcdir="BINDINGS/python/python-ecore",
    dependencies={"common": ["efl", "python-evas"]},
    # generated code sucks, no way to change it:
    env={"common": {"CFLAGS": "-Wno-unused-parameter -Wno-unused-but-set-variable -Wno-shadow -Wno-strict-aliasing"}},
    configure_flags={
        "common": [
            "--enable-ecore-file",
            "--enable-ecore-evas",
            ],
        "linux": [
            "--enable-ecore-x",
            "--enable-ecore-imf",
            ],
        "win": [
            "--enable-ecore-win32",
            ],
        },
    )

Package(
    name="python-edje",
    srcdir="BINDINGS/python/python-edje",
    dependencies={"common": ["edje", "python-evas"]},
    # generated code sucks, no way to change it:
    env={"common": {"CFLAGS": "-Wno-unused-parameter -Wno-unused-but-set-variable -Wno-shadow -Wno-strict-aliasing"}},
    )

Package(
    name="python-e_dbus",
    srcdir="BINDINGS/python/python-e_dbus",
    exclusive_platforms=["linux"],
    dependencies={"common": ["e_dbus", "python-evas"]},
    )
