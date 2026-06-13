# morf

`morf` is a high-performance, native C++ command-line utility and visual desktop workspace designed to bring intelligent version control, diffing, and interactive merging to 3D meshes. 

Engineered specifically to integrate seamlessly with standard **Git** and **Git LFS (Large File Storage)** pipelines, `morf` treats 3D geometry as code—allowing developers, technical artists, and automated pipelines to isolate structural changes, evaluate modifications down to the micrometer, and resolve mesh merge conflicts without leaving their terminal workflow.

---

## Key Features

* **Blazing Fast Spatial Diffing:** Leverages highly optimized C++ standard library structures and KD-Tree spatial indexing to evaluate massive 3D models in milliseconds.
* **Intelligent Geometric Classification:** Automatically breaks down file differences into clear, deterministic geometric categories:
  * <kbd>🟢 Green</kbd> **Added Geometry:** Entirely new faces or component groups.
  * <kbd>🔴 Red</kbd> **Deleted Geometry:** Missing components or removed face structures.
  * <kbd>🟡 Yellow</kbd> **Deformation Tracking:** Micro-movements where vertices shifted position, generating precise displacement telemetry.
* **Native 3D Viewport UI:** Launches an ultra-lightweight, hardware-accelerated desktop canvas (powered by Raylib and Dear ImGui) to let humans visually audit 3D changes side-by-side.
* **Native Git Integration Hooks:** Plugs directly into the system shell as a native Git extension wrapper (`git diff` and `git merge`).

---
