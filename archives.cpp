#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>

bool extract_tar_gz(const std::string& archive_path, const std::string& output_dir) {
    struct archive *a = archive_read_new();
    struct archive *ext = archive_write_disk_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    
    if (archive_read_open_filename(a, archive_path.c_str(), 10240) != ARCHIVE_OK) {
        return false;
    }

    struct archive_entry *entry;
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
    
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        std::string full_path = output_dir + "/" + archive_entry_pathname(entry);
        archive_entry_set_pathname(entry, full_path.c_str());
        
        std::string dirpath = output_dir + "/" + archive_entry_pathname(entry);
        dirpath = dirpath.substr(0, dirpath.find_last_of('/'));
        mkdir(dirpath.c_str(), 0755);

        archive_write_disk_set_options(ext, flags);
        int r = archive_write_header(ext, entry);
        if (r >= ARCHIVE_OK) {
            const void *buff;
            size_t size;
            la_int64_t offset;
            
            while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
                archive_write_data_block(ext, buff, size, offset);
            }
        }
        archive_write_finish_entry(ext);
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    return true;
}
