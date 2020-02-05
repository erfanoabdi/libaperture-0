/*
 * Copyright (C) 2020 James Westman
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      James Westman <james@flyingpimonster.net>
 */


namespace Aperture.DiskSpace {
    /**
     * Returns the amount of free space on the file's filesystem in bytes.
     */
    public async uint64 get_free_space(string path) throws Error {
        var file = File.new_for_path(path);
        FileInfo info = yield file.query_filesystem_info_async("filesystem::*");
        return info.get_attribute_uint64(FileAttribute.FILESYSTEM_FREE);
    }
}
