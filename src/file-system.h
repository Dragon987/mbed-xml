#pragma once

namespace drago {
    template<typename FSType>
    struct FileSystem {
        FSType* fs;
        BlockDevice* bd;

        FileSystem(const char* name)
        {
            fs = new FSType(name);
            bd = BlockDevice::get_default_instance();

            auto err = fs->mount(bd); 
            if (err) {
                printf("Could not mount file system, error: %s\r\n", strerror(err));
                exit(err);
            }
        }

        ~FileSystem() {
            auto err = fs->unmount(); 
            if (err) {
                printf("Could not unmount file system, error: %s\r\n", strerror(err));
                exit(err);
            }
            delete fs;
        }
    };
}