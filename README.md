# PBL2 - Quản lý và tìm kiếm gia sư

Dự án C++17 + Qt 6 Widgets + CMake.

## Cấu trúc dự án

- src/entities: entities OOP
- src/structures: Vector, HashMap
- src/services: MatchingService, business logic
- src/repositories: FileManager, dữ liệu TXT
- src/ui: Qt widgets
- data: dữ liệu mẫu dạng txt
- tests: backend test

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Mục tiêu

- Quản lý học sinh, gia sư, lớp học và hợp đồng.
- Tìm kiếm gia sư phù hợp bằng MatchingService.
- Lưu dữ liệu bằng file TXT.
