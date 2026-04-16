# <% tp.file.title %>

## Overview
<% tp.system.prompt("Briefly describe the purpose of this class") %>

## Responsibilities
- <% tp.system.prompt("Responsibility 1") %>
- <% tp.system.prompt("Responsibility 2") %>
- <% tp.system.prompt("Responsibility 3 (optional)") %>

## Public Interface
```cpp
<% tp.file.cursor() %>
```

## Relationships
### Uses
- [[<% tp.system.prompt("Class or data structure this uses") %>]]

### Used by
- [[<% tp.system.prompt("Engine or subsystem that owns this class") %>]]

## Data Flow
- Inputs: <% tp.system.prompt("Main inputs") %>
- Outputs: <% tp.system.prompt("Main outputs") %>

## Notes
- Threading:
- Real-time constraints:
- Safety considerations:
