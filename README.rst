
# Mesa 3D Graphics Library
[Mesa 3D Official Website](https://mesa3d.org)

## Panfork mod with Bifrost "kbase shim" Support Branch
This branch provides partial support for Mali gXX GPUs and limited support for tXX models.

### Compiling:

To compile Mesa with support for the Panfrost drivers, use the following commands:

```bash
CFLAGS="-O3" meson -Dgallium-drivers=panfrost,swrast \
                    -Dvulkan-drivers= \
                    -Dbuildtype=release \
                    -Dllvm=disabled \
                    -Dprefix=/opt/panfrost

ninja -j8 install

System Configuration:

After installation, add the /opt/panfrost prefix to ensure the system loads the correct libraries:

echo /opt/panfrost/lib/aarch64-linux-gnu | sudo tee /etc/ld.so.conf.d/0-panfrost.conf

sudo ldconfig

Testing:

Install glmark2 to test the setup:

sudo apt install glmark2-es2-x11 -y

Run the following test command, replacing $YOURGPUID with the ID of your GPU:

DISPLAY=:1 GALLIUM_DRIVER=panfrost PAN_GPU_ID=$YOURGPUID glmark2-es2

GPU ID List:

Remove the 0x prefix from your GPU ID before using it in the command. For example, if your GPU ID is 0x7212, you should use 7212.

This version should be clean and properly formatted for GitHub.

