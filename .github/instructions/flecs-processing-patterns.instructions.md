---
description: "Use when designing Flecs feature processing strategies, especially features involving relationship-scoped or inner queries, entity iteration, and single-pass versus multi-phase system updates."
---
# Flecs Processing Patterns

- Choose the entity-processing strategy that matches the feature's relationships, data access, ordering requirements, and expected workload.
- When repeatedly iterating entities partitioned by a relationship target (for example, children grouped by parent), consider one reusable cached query with `group_by` rather than creating a separate inner query for each target.
- Select the relevant group for each target when iterating. Avoid rebuilding queries or scanning all entities when a query can express the intended partition.
- `EcsChildOf` and `EcsParent` are distinct hierarchy storages. `EcsChildOf` is a fragmenting relationship: its parent target is part of the table type, so `group_by = EcsChildOf` can place those tables in the group keyed by that parent. `EcsParent` stores the parent in a component and does not fragment tables by parent. A query grouped by `EcsChildOf` can therefore return `EcsParent`-stored children in group `0`, alongside other matching tables that have no fragmenting `ChildOf` target. If the query must support both hierarchy types, iterate the requested parent's group and group `0`, then verify `ecs_get_parent` before processing every result from group `0`:

  ```c
  ecs_query_t *bars = ecs_query(world, {
      .cache_kind = EcsQueryCacheAuto,
      .group_by = EcsChildOf,
      .terms = {
          { .id = ecs_id(Bar), .inout = EcsInOut },
          { ecs_childof(EcsWildcard), .inout = EcsInOutFilter }
      }
  });

  // In the system that processes each Foo parent:
  uint64_t groups[] = { foo, 0 };
  for (int g = 0; g < 2; g ++) {
      ecs_iter_t inner = ecs_query_iter(it->stage, bars);
      ecs_iter_set_group(&inner, groups[g]);
      while (ecs_query_next(&inner)) {
          Bar *bar = ecs_field(&inner, Bar, 0);
          for (int i = 0; i < inner.count; i ++) {
              if (ecs_get_parent(it->world, inner.entities[i]) != foo) {
                  continue;
              }
              // Process bar[i], which belongs to this Bar child of Foo.
          }
      }
  }
  ```

  Group `0` can contain children belonging to many parents (and entities without a parent), so always verify the direct parent before processing those results. `EcsParent` queries can use `group_by = EcsParentDepth` to group by hierarchy depth, but that does not group by parent. Also, Flecs documents that `Or` and `Not` query operators do not yet work with `EcsParent` hierarchies; don't rely on those operators to filter Parent-stored entities or their ancestors. Reuse the same query across parents and frames.
- Prefer a single pass when it can perform the work correctly. Use distinct reset and update phases when required for correctness, ordering, or system scheduling; do not add or remove phases solely to follow a blanket rule.
- Verify that queries match the intended entities and preserve required iteration semantics.
- Treat performance measurements as workload-specific evidence, not a universal rule. For performance-sensitive changes, benchmark representative workloads and verify behavioral equivalence.
