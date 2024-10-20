Mesa 3D Graphics Library
=========================
`Mesa 3D Official Website <https://mesa3d.org>`_

Panfork Bifrost/midgard/valhall Support Branch
------------------------------------
This branch provides partial support for Mali gXX GPUs and some tXX models.

Compiling
---------
To compile Mesa with support for the Panfrost driver, run the following commands:

.. code-block:: bash

    CFLAGS="-O3" meson -Dgallium-drivers=panfrost,swrast \
                        -Dvulkan-drivers= \
                        -Dbuildtype=release \
                        -Dllvm=disabled \
                        -Dprefix=/opt/panfrost

    ninja -j8 install

System Configuration
--------------------
After compiling and installing, configure the system to load the correct libraries by adding the installation path to the dynamic linker configuration:

.. code-block:: bash

    echo /opt/panfrost/lib/aarch64-linux-gnu | sudo tee /etc/ld.so.conf.d/0-panfrost.conf
    sudo ldconfig

Testing
-------
To test the installation, install `glmark2-es2` and run the test for your GPU:

.. code-block:: bash

    sudo apt install glmark2-es2-x11 -y
DISPLAY=:1 GALLIUM_DRIVER=panfrost glmark2-es2
