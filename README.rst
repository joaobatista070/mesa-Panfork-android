Mesa 3D Graphics Library
=========================
`Mesa 3D Official Website <https://mesa3d.org>`_

Panfork Bifrost/midgard "kbase shim" Support Branch
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

    DISPLAY=:1 GALLIUM_DRIVER=panfrost PAN_GPU_ID=$YOURGPUID glmark2-es2

GPU ID List
-----------
Remove the `0x` prefix from your GPU ID before using it in the command. For example, if your GPU ID is `0x7212`, you should use `7212`.

+-----------------+----------------+
| GPU ID          | GPU Name       |
+-----------------+----------------+
| 0x720, 0x750    | T720, T760     |
| 0x820, 0x830    | T820, T830     |
| 0x860, 0x880    | T860, T880     |
| 0x6000, 0x6221  | G71, G72       |
| 0x7090, 0x7093  | G51, G31       |
| 0x7211, 0x7212  | G76, G52       |
| 0x7402, 0x9093  | G52-r1, G57    |
| 0xa867, 0xa802  | G610, G710     |
+-----------------+----------------+
