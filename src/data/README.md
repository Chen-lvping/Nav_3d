# Runtime data

This directory is a local convenience location. Generated maps, bags, point
clouds, and trajectories are not source code and are excluded from Git.

Recommended layout:

```text
point_cloud/scans.pcd
trace_data/mapping_trajectory.txt
traversable/traversable_areas.pcd
bags/
```

Set `NAV3D_DATA_ROOT` to this directory or to external storage. Large datasets
should be published as versioned release assets, object storage, or a separate
Git LFS dataset with checksums and collection metadata.

Do not commit credentials, device serial numbers, private network topology, or
recordings containing sensitive environment data.
