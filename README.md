# Smart Campus Parcel Collection and Delivery Management System

CT077-3-2-DSTR Assignment 2. One integrated C++ program with five modules that share the same parcel data. All data structures are written by hand (no STL vector, list, stack, queue, priority_queue or deque).

## Compile and run

From the repository root:

```
g++ -std=c++17 main.cpp -o parcel_system
./parcel_system
```

Run it from the repository root, because it reads and writes the files in `data/`. It also compiles with `-std=c++11`.

At start-up the program asks for a Staff ID (for example `ST01` or `ST02`). This ID is stored on every activity in the history.

Each task saves its data after every change. Choose `[0] Save and exit` to save all four data files once more before closing.

## Modules

| Menu | Module | Owner | Folder | Data structure |
|---|---|---|---|---|
| 1 | Parcel intake, sorting and processing | Isac Lim | `task1 (Isac Lim)/` | Queue (intake) + Priority Queue (processing) |
| 2 | Service and collection requests | Soh Ye Wen | `task2 (Soh Ye Wen)/` | Sorted linked list (priority order) |
| 3 | Collection counters and delivery resources | Ian Chin | `task3 (Ian Chin)/` | Circular Queue (round-robin per resource type) |
| 4 | Activity history and undo | Muhlis | `task4 (Muhlis)/` | Stack |
| 5 | Parcel category organisation | Kerjinn | `task5 (kerjinn)/` | Binary Search Tree (key: Parcel ID) |

Each task folder also has its own `main.cpp` to test that module on its own. Compile and run it from inside that folder, because it uses `../data/`:

```
cd "task3 (Ian Chin)"
g++ -std=c++17 main.cpp -o task3
./task3
```

## Data files

| File | Contents |
|---|---|
| `data/parcels.txt` | Parcels (Task 1, also edited by Task 5) |
| `data/requests.txt` | Service requests (Task 2) |
| `data/resources.txt` | Counters and delivery personnel (Task 3) |
| `data/activities.txt` | Activity history (Task 4) |

Each file starts with `#` comment lines that explain its columns and codes.

## Sample data for a demo

- P001 to P011 cover every parcel status except Cancelled. P009 and P010 are Received and P008 is Pending, ready to sort and process.
- R005, R006 and R007 are Pending requests with different service types, to show the next-request rule.
- S01 and DP02 are busy, DP01 is Unavailable and C03 is in Maintenance, to show the error cases. C01, C02, S02, D01 and DP03 are free for new jobs.
- The latest undoable activity is A0044 (P011 reported Missing).

A typical run: register a parcel (1), sort and process it (1), create and process a request (2), finish it (2), check the resource and utilisation report (3), trace the parcel and try undo (4), then search it in the tree (5).
