"""
PlatformIO pre-build script to inject version information at build time.

This script runs before compilation and injects:
- BUILD_SW_VERSION: Date-based software version in YYYY.MM.DD format (UTC)
- BUILD_HW_VERSION: Hardware version string based on board type (esp32c6 or esp32s3)

These defines are used in pub_sub_conn.cpp to populate the MQTT discovery device JSON.
"""

from datetime import datetime, timezone

Import("env")

# Generate software version from current UTC date
build_date_utc = datetime.now(timezone.utc)
sw_version = build_date_utc.strftime("%Y.%m.%d")

# Determine hardware version from PlatformIO environment name
env_name = env['PIOENV']
if 'esp32c6' in env_name.lower():
    hw_version = "esp32c6"
elif 'esp32s3' in env_name.lower():
    hw_version = "esp32s3"
else:
    raise ValueError(
        f"ERROR: Unknown board type in environment '{env_name}'. "
        f"Expected environment name to contain 'esp32c6' or 'esp32s3'. "
        f"Please update build_version.py to support this board type."
    )

# Inject version defines into build
env.Append(CPPDEFINES=[
    ('BUILD_SW_VERSION', f'\\"{sw_version}\\"'),
    ('BUILD_HW_VERSION', f'\\"{hw_version}\\"')
])

# Print build info for verification
print(f"=== Build Version Info ===")
print(f"  Environment: {env_name}")
print(f"  SW Version:  {sw_version}")
print(f"  HW Version:  {hw_version}")
print(f"  Build Time:  {build_date_utc.strftime('%Y-%m-%d %H:%M:%S UTC')}")
print(f"==========================")
