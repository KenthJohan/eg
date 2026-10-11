# Flecs Hierarchy Traversal: One Level vs Multiple Levels

Reference example: [examples/parentmatch/src/main.c](examples/parentmatch/src/main.c)

## Multiple levels (any ancestor)

```c
{.id = ecs_id(Comp), .trav = EcsChildOf, .src.id = EcsUp}
```

`EcsUp` with `.trav = EcsChildOf` walks the whole parent chain and matches the nearest ancestor that has `Comp`. It is not limited to the direct parent. The matched component is read with `ecs_field_shared(...)`.

Use it when the data is inherited down the tree:

- The value is a property of a whole subtree (for example a parent `EgShapesRectangle` used as bounds, or `EcsDisabled` on any ancestor disabling all descendants).
- It does not matter how deeply the entity is nested.
- Intermediate entities are plain containers.

## One level (direct parent only)

```c
{.id = ecs_id(Comp), .src = {.id = EcsIsVariable, .name = "parent"}},
{.first.id = EcsChildOf, .second = {.id = EcsIsVariable, .name = "parent"}, .inout = EcsInOutNone},
```

Use a variable as the source of `Comp` and tie it to the entity with `(ChildOf, $parent)`. The variable can only be the direct parent, so grandparents never match.

Rules:

- Both terms must use the same variable name.
- Put the `(ChildOf, $parent)` term anywhere in the query. It does not change the `ecs_field` indices of other terms except by adding one index. Append it last to keep existing indices.
- Use `.inout = EcsInOutNone` on the `ChildOf` term because it has no data.
- The component on `$parent` is still read with `ecs_field_shared(...)` (it is not `self`).

Use it when the component describes how the *direct* children are laid out or controlled, and a nested descendant must not be affected:

- Layout owners such as `EgUiFlowsFlow` and `EgUiFreeformsLayout`. A child of a Flow is positioned by that Flow, not by a Layout further up.
- Two systems that handle the same child differently depending on its parent type. With `EcsUp` both can match the same entity (an ancestor Layout plus a parent Flow) and apply the change twice.

## Pitfall that motivated this

`EgUiBoundsResizable_ApplyFlow` and `EgUiBoundsResizable_ApplyFreeforms` both matched Flow children because `root2d` has a `Layout` ancestor and `EcsUp` finds it. The resize was applied twice per frame, so a held edge vibrated between two sizes. Matching the layout component on the direct parent only fixes this.

## Choosing

| Question | Use |
| --- | --- |
| Is the value inherited by the whole subtree? | `EcsUp` (multiple levels) |
| Does the component control how its direct children behave? | Variable + `(ChildOf, $parent)` (one level) |
| Could two systems match the same entity through different ancestors? | One level |
