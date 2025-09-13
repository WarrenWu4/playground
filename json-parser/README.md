# Exploring JSON Parsing

Efficient read & write operations

- lookup tables
- conditional moves
- bit manipulation
- little bit of SIMD

---
---

Customizable object detection & sanitization

Ex. 

```json
{
    "red": "RGB(255, 0, 0)"
}
```

gets translated to something like 

```cpp
COLOR red = RGB(255, 0, 0);
```
