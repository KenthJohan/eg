---
description: "Use when designing Flecs feature processing strategies, especially features involving relationship-scoped or inner queries, entity iteration, and single-pass versus multi-phase system updates."
---
# Flecs Processing Patterns

- Choose the entity-processing strategy that matches the feature's relationships, data access, ordering requirements, and expected workload.
- When repeatedly iterating entities partitioned by a relationship target (for example, children grouped by parent), consider one reusable cached query with `group_by` rather than creating a separate inner query for each target.
- Select the relevant group for each target when iterating. Avoid rebuilding queries or scanning all entities when a query can express the intended partition.
- A `group_by = EcsChildOf` query may need to handle both fragmenting and non-fragmenting child storage. Fragmenting children are in the group keyed by their parent; non-fragmenting children can be in group `0`. Iterate both groups and filter group `0` results by their actual parent:

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

  Group `0` can contain children belonging to many parents, so always verify the direct parent before processing those results. Reuse the same query across parents and frames.
- Prefer a single pass when it can perform the work correctly. Use distinct reset and update phases when required for correctness, ordering, or system scheduling; do not add or remove phases solely to follow a blanket rule.
- Verify that queries match the intended entities and preserve required iteration semantics.
- Treat performance measurements as workload-specific evidence, not a universal rule. For performance-sensitive changes, benchmark representative workloads and verify behavioral equivalence.
