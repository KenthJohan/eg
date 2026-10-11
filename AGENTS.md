# Flecs Hierarchy Traversal: One Level vs Multiple Levels

Reference example: [examples/parentmatch/src/main.c](examples/parentmatch/src/main.c)

## Multiple levels (any ancestor)

```c
{.id = ecs_id(Comp), .trav = EcsChildOf, .src.id = EcsUp}
```

`EcsUp` with `.trav = EcsChildOf` walks the whole parent chain and matches the nearest ancestor that has `Comp`. It is not limited to the direct parent. Read the matched component with `ecs_field_shared(...)`.

Use it when the data is inherited down the tree:

- The value is a property of a whole subtree.
- Nesting depth does not matter.
- Intermediate entities are plain containers.

## One level (direct parent only)

```c
{.id = ecs_id(Comp), .src = {.id = EcsIsVariable, .name = "parent"}},
{.first.id = EcsChildOf, .second = {.id = EcsIsVariable, .name = "parent"}, .inout = EcsInOutNone},
```

Use a variable as the source of `Comp` and tie it to the entity with `(ChildOf, $parent)`. The variable can only be the direct parent, so grandparents never match.

Rules:

- Both terms must use the same variable name.
- The `(ChildOf, $parent)` term adds one term. Append it last so existing `ecs_field` indices do not shift.
- Use `.inout = EcsInOutNone` on the `ChildOf` term because it has no data.
- Read the component on `$parent` with `ecs_field_shared(...)`, not `self`.

Use it when the component describes how the *direct* children are handled, and deeper descendants must not be affected:

- The parent component is a controller or owner of its immediate children.
- Different systems handle the same child differently depending on the type of its parent.

## Pitfall

If two systems each look for a different parent component with `EcsUp`, one entity can match both through different ancestors, and both systems then act on it. Symptoms are duplicated updates, doubled deltas, or values that oscillate between frames. Matching the controlling component on the direct parent only avoids this.

## Choosing

| Question | Use |
| --- | --- |
| Is the value inherited by the whole subtree? | `EcsUp` (multiple levels) |
| Does the component control how its direct children behave? | Variable + `(ChildOf, $parent)` (one level) |
| Could two systems match the same entity through different ancestors? | One level |
