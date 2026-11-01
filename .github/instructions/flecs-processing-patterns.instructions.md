---
description: "Use when designing Flecs feature processing strategies, especially features involving relationship-scoped or inner queries, entity iteration, and single-pass versus multi-phase system updates."
---
# Flecs Processing Patterns

- Choose the entity-processing strategy that matches the feature's relationships, data access, ordering requirements, and expected workload.
- When repeatedly iterating entities partitioned by a relationship target (for example, children grouped by parent), consider one reusable cached query with `group_by` rather than creating a separate inner query for each target.
- Select the relevant group for each target when iterating. Avoid rebuilding queries or scanning all entities when a query can express the intended partition.
- Prefer a single pass when it can perform the work correctly. Use distinct reset and update phases when required for correctness, ordering, or system scheduling; do not add or remove phases solely to follow a blanket rule.
- Verify that queries match the intended entities and preserve required iteration semantics.
- Treat performance measurements as workload-specific evidence, not a universal rule. For performance-sensitive changes, benchmark representative workloads and verify behavioral equivalence.
