const std = @import("std");
const testing = std.testing;

const MAX_PATH = 260;

// TODO: move to std.mem or see if this already exists
pub fn replaceElem(comptime T: type, slice: []T, needle: T, replacement: T) void {
    for (slice) |e, i| {
        if (e == needle) {
            slice[i] = replacement;
        }
    }
}

// TODO: move this to std.mem?
/// Collapses consecutive duplicate elements into one entry.
pub fn collapseRepeats(comptime T: type, slice: []T, needle: T) usize {
    if (slice.len == 0) return 0;
    var write_idx : usize = 1;
    var read_idx : usize = 1;
    while (read_idx < slice.len) : (read_idx += 1) {
        if (slice[read_idx - 1] != needle or slice[read_idx] != needle) {
            slice[write_idx] = slice[read_idx];
            write_idx += 1;
        }
    }
    return write_idx;
}

fn testCollapseRepeats(str: []const u8, needle: u8, expected: []const u8) !void {
   const mutable = try std.testing.allocator.dupe(u8, str);
   defer std.testing.allocator.free(mutable);
   const actual = mutable[0 .. collapseRepeats(u8, mutable, needle)];
   testing.expect(std.mem.eql(u8, actual, expected));
}
test "collapseRepeats" {
    try testCollapseRepeats(""   , '/', "");
    try testCollapseRepeats("a"  , '/', "a");
    try testCollapseRepeats("/"  , '/', "/");
    try testCollapseRepeats("//" , '/', "/");
    try testCollapseRepeats("/a"  , '/', "/a");
    try testCollapseRepeats("//a" , '/', "/a");
    try testCollapseRepeats("a/"  , '/', "a/");
    try testCollapseRepeats("a//" , '/', "a/");
    try testCollapseRepeats("a/a" , '/', "a/a");
    try testCollapseRepeats("a//a", '/', "a/a");
    try testCollapseRepeats("//a///a////", '/', "/a/a/");
}

pub const RemoveDotDirsError = error { TooManyParentDirs };

// Removes '.' and '..' path components from a path that has already been sanitized.
// A "sanitized" path means that:
//    1) all forward slashes have been replaced with back slashes
//    2) all repeating back slashes have been collapsed
pub fn removeDotDirsSanitized(comptime T: type, path: []T) RemoveDotDirsError!usize {
    if (path.len > 0 and path[0] == '\\') {
        return 1 + try removeDotDirsSanitizedRelative(T, path[1..]);
    }
    return removeDotDirsSanitizedRelative(T, path);
}

fn removeDotDirsSanitizedRelative(comptime T: type, path: []T) RemoveDotDirsError!usize {
    std.debug.assert(path.len == 0 or path[0] != '\\');
    
    var write_idx : usize = 0;
    var read_idx : usize = 0;
    while (read_idx < path.len) {
        if (path[read_idx] == '.') {
            if (read_idx + 1 == path.len)
                return write_idx;

            const after_dot = path[read_idx + 1];
            if (after_dot == '\\') {
                read_idx += 2;
                continue;
            }
            if (after_dot == '.' and (read_idx + 2 == path.len or path[read_idx + 2] == '\\') ) {
                if (write_idx == 0) return error.TooManyParentDirs;
                std.debug.assert(write_idx >= 2);
                write_idx -= 1;
                while (true) {
                    write_idx -= 1;
                    if (write_idx == 0 or path[write_idx] == '\\') break;
                }
                if (read_idx + 2 == path.len)
                    return write_idx;
                read_idx += 3;
                continue;
            }
        }

        // skip to the next path separator
        while (true) : (read_idx += 1) {
            if (read_idx == path.len)
                return write_idx;
            path[write_idx] = path[read_idx];
            write_idx += 1;
            if (path[read_idx] == '\\')
                break;
        }
        read_idx += 1;
    }
    return write_idx;
}

fn testRemoveDotDirs(str: []const u8, expected: []const u8) !void {
   const mutable = try std.testing.allocator.dupe(u8, str);
   defer std.testing.allocator.free(mutable);
   const actual = mutable[0 .. try removeDotDirsSanitized(u8, mutable)];
   std.debug.warn("-------\ntest '{}'\nexpt '{}'\nactl '{}'\n", .{ str, expected, actual });
   testing.expect(std.mem.eql(u8, actual, expected));
}
fn testRemoveDotDirsError(err: anyerror, str: []const u8) !void {
   const mutable = try std.testing.allocator.dupe(u8, str);
   defer std.testing.allocator.free(mutable);
   std.debug.warn("-------\ntest '{}' error {}\n", .{str, err});
   testing.expectError(err, removeDotDirsSanitized(u8, mutable));
}
test "removeDotDirs" {
    inline for ([_][]const u8 { "", "\\" }) |prefix| {
        try testRemoveDotDirs(prefix ++ "", prefix ++ "");
        try testRemoveDotDirs(prefix ++ ".", prefix ++ "");
        try testRemoveDotDirs(prefix ++ ".\\", prefix ++ "");
        try testRemoveDotDirs(prefix ++ ".\\.", prefix ++ "");
        try testRemoveDotDirs(prefix ++ ".\\.\\", prefix ++ "");
        try testRemoveDotDirs(prefix ++ ".\\.\\.", prefix ++ "");
    
        try testRemoveDotDirs(prefix ++ "a", prefix ++ "a");
        try testRemoveDotDirs(prefix ++ "a\\", prefix ++ "a\\");
        try testRemoveDotDirs(prefix ++ "a\\b", prefix ++ "a\\b");
        try testRemoveDotDirs(prefix ++ "a\\.", prefix ++ "a\\");
        try testRemoveDotDirs(prefix ++ "a\\b\\.", prefix ++ "a\\b\\");
        try testRemoveDotDirs(prefix ++ "a\\.\\b", prefix ++ "a\\b");

        try testRemoveDotDirs(prefix ++ ".a", prefix ++ ".a");
        try testRemoveDotDirs(prefix ++ ".a\\", prefix ++ ".a\\");
        try testRemoveDotDirs(prefix ++ ".a\\.b", prefix ++ ".a\\.b");
        try testRemoveDotDirs(prefix ++ ".a\\.", prefix ++ ".a\\");
        try testRemoveDotDirs(prefix ++ ".a\\.\\.", prefix ++ ".a\\");
        try testRemoveDotDirs(prefix ++ ".a\\.\\.\\.b", prefix ++ ".a\\.b");
        try testRemoveDotDirs(prefix ++ ".a\\.\\.\\.b\\", prefix ++ ".a\\.b\\");

        try testRemoveDotDirsError(error.TooManyParentDirs, prefix ++ "..");
        try testRemoveDotDirsError(error.TooManyParentDirs, prefix ++ "..\\");
        try testRemoveDotDirsError(error.TooManyParentDirs, prefix ++ ".\\..\\");
        try testRemoveDotDirsError(error.TooManyParentDirs, prefix ++ ".\\.\\..\\");

        try testRemoveDotDirs(prefix ++ "a\\..", prefix ++ "");
        try testRemoveDotDirs(prefix ++ "a\\..\\", prefix ++ "");
        try testRemoveDotDirs(prefix ++ "a\\..\\.", prefix ++ "");
        try testRemoveDotDirs(prefix ++ "a\\..\\.\\", prefix ++ "");
        try testRemoveDotDirs(prefix ++ "a\\..\\.\\.", prefix ++ "");
        try testRemoveDotDirsError(error.TooManyParentDirs, prefix ++ "a\\..\\.\\.\\..");

        try testRemoveDotDirs(prefix ++ "a\\..\\.\\.\\b", prefix ++ "b");
        try testRemoveDotDirs(prefix ++ "a\\..\\.\\.\\b\\", prefix ++ "b\\");
        try testRemoveDotDirs(prefix ++ "a\\..\\.\\.\\b\\.", prefix ++ "b\\");
        try testRemoveDotDirs(prefix ++ "a\\..\\.\\.\\b\\.\\", prefix ++ "b\\");
        try testRemoveDotDirs(prefix ++ "a\\..\\.\\.\\b\\.\\..", prefix ++ "");
        try testRemoveDotDirs(prefix ++ "a\\..\\.\\.\\b\\.\\..\\", prefix ++ "");
        try testRemoveDotDirs(prefix ++ "a\\..\\.\\.\\b\\.\\..\\.", prefix ++ "");
        try testRemoveDotDirsError(error.TooManyParentDirs, prefix ++ "a\\..\\.\\.\\b\\.\\..\\.\\..");
    }
}


// Step 1: convert all forward slashes to back slashes
// Step 2: collapse duplicate backslashes
// Step 3: remove '.' and '..' directory parts
// Step 4: remove trailing slash if applicable
pub fn toNtPath(comptime T: type, path: []T) !usize {
    replaceElem(T, path, '/', '\\');
    var new_len = collapseElem(T, path, '\\');
    new_len = removeDotDirsSanitized(T, path[0 .. new_len]);
    if (new_len > 0 and path[new_len] == '\\') new_len -= 1;
    return new_len;
}
fn testToNtPath(path: []const u8, expected: []const u8) !void {
   {
       var buf : [MAX_PATH]u8 = undefined;
       std.mem.copy(u8, &buf, path);
       const mutable_path = buf[0 .. path.len];
       const actual = buf[0 .. try toNtPath(u8, mutable_path)];
       testing.expect(std.mem.eql(u8, actual, expected));
   }
   {
       var buf : [MAX_PATH]u16 = undefined;
       var expected_w_buf : [MAX_PATH]u16 = undefined;
       const wpath = buf[0 .. try std.unicode.utf8ToUtf16Le(&buf, path)];
       const expected_w = expected_w_buf[0 .. try std.unicode.utf8ToUtf16Le(&expected_w_buf, expected)];
       const actual = buf[0 .. try toNtPath(u16, wpath)];
       testing.expect(std.mem.eql(u16, actual, expected_w));
   }
}
test "toNtPath" {
    //try testToNtPath("");

//    try testToNtPath("a/b", "a\\b");
//    try testToNtPath("a/b/", "a\\b\\");
//    try testToNtPath("/a/b/", "\\a\\b\\");
}

