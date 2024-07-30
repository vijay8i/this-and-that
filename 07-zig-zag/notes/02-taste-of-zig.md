# A taste of Zig

> These notes were taken from this talk by Andrew Kelley, who is the creator of
> the Zig Programming Language.

Why:

- No esoteric language features.
- Let's you focus on code that is about your product than about language usage

Examples:

- ArrayList
- Inline for loops
- MultiArrayList
- AutoArrayHashMap
- C Translation
- Unit Testing
- Untagged Union Safety

## ArrayList

```zig
fn ArrayList(comptime T: type) type {
  return struct {
    items: []T,
    capacity: usize
  };
}
```

## Inline for loops

```zig
const std = @import("std");
cinst assert = std.debug.assert;

const Data = struct {
  foo: Foo,
  bytes: [8]u8,
  ok: bool,
};

const Foo = enum {hello, world};

pub fn main() void {
  var d: Data = .{
    .foo = .world,
    .bytes = "abc",
    .ok = true,
  }

  dump(d);
}

fn dump(data: anytype) void {
  const T = @TypeOf(data);
  inline for (@typeInfo(T).Struct.fields) |field| {
    std.debug.print("{any}\n", .{@field(data, field.name)})
  }
}
```

## AutoArrayHashMap
