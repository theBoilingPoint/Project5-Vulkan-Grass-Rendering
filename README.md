Vulkan Grass Rendering
==================================

**University of Pennsylvania, CIS 565: GPU Programming and Architecture, Project 5**

* Xinran Tao
  * [LinkedIn](https://www.linkedin.com/in/xinran-tao/), [GitHub](https://github.com/theBoilingPoint), [Portfolio](https://www.xinrantao.com/)
* Tested on: Windows 11 Enterprise, AMD Ryzen 7 7800X3D 8 Core Processor @ 4.201GHz, RTX 2080Ti (Personal PC)

# Introduction
In this project, I am implementing a grass simulation introduced by [Responsive Real-Time Grass Rendering for General 3D Scenes](https://doi.org/10.1145/3023368.3023380) in [Vulkan](https://www.vulkan.org/). 

The features include:
- Grass blades are rendered with gravity, recovery, and wind forces applied.
- Grass blades maintain realistic shapes when forces are applied.
- Grass blades are culled by orientation, view frustum, and distance from the camera.
- Grass blades tessellation levels are determined by the distance from the camera. The closer they are to the camera, the higher the tessellation levels.

# Showcase
|![](img/my/grass_without_forces.gif)|
|:--:|
|**Stage 1:** Grass Rendering without Forces|

|![](img/my/grass_with_forces.gif)|
|:--:|
|**Stage 2:** Grass Rendering with Forces|

|![](img/my/grass_with_tesslv_col.gif)|
|:--:|
|**Stage 3:** Grass Rendering with Varying Tessellation Levels|
|*The brighter the colour, the higher the tessellation level.*|

# Performance Analysis
>To analyse the performance, I use the `VK_LAYER_LUNARG_monitor` layer to measure the FPS. I did this by adding the following code to the `Instance.cpp` file:
> ```cpp
> Instance::Instance(const char* applicationName, unsigned int additionalExtensionCount, > const char** additionalExtensions) {
>     ...
> 
>     /** Note that I didn't check if this layer exists! **/
>     const char* instance_layers[] = { "VK_LAYER_LUNARG_monitor" };
>     createInfo.enabledLayerCount = 1;
>     createInfo.ppEnabledLayerNames = instance_layers;
>     /*******************************************************/
>
>     ...
> }
> ```
> As stated in the comment, I didn't check if the extension exists. Please be mindful about this when running the code.

## Handling Increasing Numbers of Grass Blades
|![](img/my/FPS%20-%20Without%20Culling%20and%20With%20Culling.png)|
|:--:|
|**FPS Comparison:** Without Culling and With Culling|

As the number of grass blades increases, the FPS declines significantly, revealing the renderer’s sensitivity to the computational load. Rendering each blade requires calculating transformations, applying shaders, and performing memory operations. With each additional blade, the rendering time per frame increases, causing the FPS to drop substantially.

For instance, with 2^10 blades, the renderer achieves high FPS values around 11,000-12,000, indicating efficient handling of the workload. However, as the blade count doubles to 2^12, the FPS decreases by about 58%, ranging between 5,000-7,000 FPS. By 2^14 blades, FPS drops further by 69%, reaching values around 1,500-3,200. At very high blade counts, such as 2^18 and 2^20 blades, FPS falls to below 200, with the lowest values at around 30 FPS, representing an additional 93% decrease. These results highlight the renderer's challenges in handling high blade counts, as the computational requirements grow exponentially, leading to severe FPS reductions.

### Why Culling Improves Performance

Culling improves performance by reducing the number of blades that the renderer processes each frame. Instead of calculating transformations, shading, and visibility for every blade in the scene, culling allows the renderer to ignore certain blades based on specific criteria (such as visibility or distance from the camera). This reduction in workload directly translates into improved FPS, as fewer blades mean fewer computations and less memory access per frame.

Each culling technique applies different criteria to determine which blades can be safely ignored, which influences how effectively each method reduces the workload. In particular:
- **Orientation Culling** discards blades facing away from the camera, as they are likely not visible to the viewer. This reduces the number of blades processed based on their orientation relative to the camera, cutting down on the processing time for blades that contribute minimally or not at all to the final image.
- **View Frustum Culling** focuses on blades outside the camera’s view frustum (the visible area in 3D space), discarding those that fall outside the field of view. This method allows the renderer to ignore blades that cannot be seen, concentrating only on visible ones and thereby improving performance.
- **Distance Culling** removes blades beyond a certain distance from the camera. Distant blades contribute less detail due to their smaller screen size, so culling them has minimal impact on visual fidelity while reducing the workload significantly.

By selectively excluding blades based on these criteria, each culling technique reduces the number of computations per frame, directly boosting the FPS.

## Performance Comparison of Culling Methods
|![](img/my/FPS%20-%20Orientation%20Culling,%20View%20Frustrum%20Culling%20and%20Distance%20Culling.png)|
|:--:|
|**FPS Comparison:** Orientation Culling, View Frustrum Culling and Distance Culling|

The data demonstrates that each culling method offers performance benefits, though the effectiveness of each method varies depending on the number of blades and the criteria used:

1. **Orientation Culling**:
   - Orientation Culling provides a modest performance improvement by discarding blades not facing the camera. At lower blade counts, the effect is limited because most blades are still within the camera's view and orientation. For instance, with 2^10 blades, Orientation Culling results in an FPS decrease of only about 5% compared to no culling, as relatively few blades are excluded.
   - As blade count increases, Orientation Culling becomes slightly more beneficial. At 2^14 blades, Orientation Culling achieves 2,151 FPS, which is approximately 40% higher than without culling. However, this method has diminishing returns with further increases in blade count. By 2^16 blades, it only provides a 22% improvement over rendering without culling, as a large number of blades are still in view and facing the camera.

2. **View Frustum Culling**:
   - View Frustum Culling provides a slightly higher improvement than Orientation Culling because it discards all blades outside the visible area, regardless of their orientation. This allows the renderer to ignore a larger portion of the scene, particularly when the camera is focused on a specific area.
   - At 2^14 blades, View Frustum Culling yields around 1,705 FPS, which is 10% lower than Orientation Culling, likely because blades outside the frustum are still limited at that range. As blade count increases, however, this method becomes more effective. For instance, at 2^16 blades, it achieves 454 FPS, which is approximately 13% higher than without culling.
   - This method is particularly beneficial when the camera's view is focused on a narrow area, as it effectively reduces the number of off-screen blades that need to be processed.

3. **Distance Culling**:
   - Distance Culling is the most effective culling technique, especially at high blade counts, as it discards blades based on their distance from the camera. This method ensures that only nearby, detailed blades are processed, reducing workload without sacrificing significant visual detail.
   - For 2^10 blades, Distance Culling provides similar FPS (around 11,215) to Orientation and View Frustum Culling, as there aren’t many distant blades. However, at 2^14 blades, it achieves 2,249 FPS, which is 32% higher than View Frustum Culling, as it discards more blades based solely on distance.
   - At 2^16 blades, Distance Culling results in 585 FPS, an improvement of 46% compared to rendering without culling. This benefit becomes even more apparent at 2^18 blades, where Distance Culling achieves 148 FPS, 42% higher than View Frustum Culling, demonstrating its effectiveness in handling extremely high blade counts.

## Summary

As the blade count increases, the renderer's FPS declines sharply due to the increased computational workload. Culling mitigates this effect by reducing the number of blades processed, with each method offering varying levels of improvement. Orientation Culling is useful for discarding blades facing away from the camera, but it provides limited benefits at high blade counts. View Frustum Culling performs better by focusing only on visible blades, offering consistent performance gains, particularly when the camera view is narrow. Distance Culling is the most effective method, especially at high blade counts, as it removes distant blades that contribute little to the scene, resulting in a substantial 42-46% improvement at extreme blade counts. 

To optimize rendering performance effectively, combining these culling methods would allow the renderer to maintain visual fidelity while managing performance across different scene conditions. Distance Culling should be prioritized for scenes with high blade counts, while Orientation and View Frustum Culling are beneficial for moderate counts or when the camera view focuses on specific regions.