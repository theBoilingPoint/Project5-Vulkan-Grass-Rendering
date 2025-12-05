#include <vector>
#include "SwapChain.h"
#include "Instance.h"
#include "Device.h"
#include "Window.h"

namespace {
  // Specify the color channel format and color space type
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
      // VK_FORMAT_UNDEFINED indicates that the surface has no preferred format, so we can choose any
      if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
          return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
      }

      // Otherwise, choose a preferred combination
      for (const auto& availableFormat : availableFormats) {
          // Ideal format and color space
          if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
              return availableFormat;
          }
      }

      // Otherwise, return any format
      return availableFormats[0];
  }

  // Specify the presentation mode of the swap chain
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
      // Second choice
      VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
      
      for (const auto& availablePresentMode : availablePresentModes) {
          if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
              // First choice
              return availablePresentMode;
          }
          else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
              // Third choice
              bestMode = availablePresentMode;
          }
      }

      return bestMode;
  }

  // Specify the swap extent (resolution) of the swap chain
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
      // If currentExtent is valid (not max and not zero), use it
      if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
          if (capabilities.currentExtent.width > 0 && capabilities.currentExtent.height > 0) {
              return capabilities.currentExtent;
          }
      }
      
      // Fallback: query window size directly
      int width, height;
      glfwGetWindowSize(window, &width, &height);
      
      // If window size is invalid, we cannot proceed (should have been caught earlier)
      if (width <= 0 || height <= 0) {
          // Return zero to indicate failure - caller should check
          return { 0, 0 };
      }
      
      VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

      // Clamp to valid range - but only if the range is valid
      if (capabilities.maxImageExtent.width > 0 && capabilities.maxImageExtent.height > 0) {
          actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
          actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
      }

      return actualExtent;
  }
}

SwapChain::SwapChain(Device* device, VkSurfaceKHR vkSurface, unsigned int numBuffers)
  : device(device), vkSurface(vkSurface), numBuffers(numBuffers) {
    
    Create();

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(device->GetVkDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device->GetVkDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create semaphores");
    }
}

void SwapChain::Create(VkSwapchainKHR oldSwapChain) {
    auto* instance = device->GetInstance();

    // Refresh surface capabilities to get current window size (they change on resize)
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(instance->GetPhysicalDevice(), vkSurface, &surfaceCapabilities);

    // Check if window is minimized - when minimized, all extents are zero
    // We cannot create a swap chain in this state, so just return early
    // The old swap chain (if any) will remain in use
    if (surfaceCapabilities.currentExtent.width == 0 && surfaceCapabilities.currentExtent.height == 0 &&
        surfaceCapabilities.minImageExtent.width == 0 && surfaceCapabilities.minImageExtent.height == 0 &&
        surfaceCapabilities.maxImageExtent.width == 0 && surfaceCapabilities.maxImageExtent.height == 0) {
        // Window is minimized - cannot create swap chain, keep using old one
        return;
    }

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(instance->GetSurfaceFormats());
    VkPresentModeKHR presentMode = chooseSwapPresentMode(instance->GetPresentModes());
    VkExtent2D extent = chooseSwapExtent(surfaceCapabilities, GetGLFWWindow());

    // Validate extent before proceeding - must not be zero and must be within bounds
    if (extent.width == 0 || extent.height == 0) {
        throw std::runtime_error("Cannot create swap chain with zero extent");
    }
    
    // Ensure extent is within the valid range
    if (extent.width < surfaceCapabilities.minImageExtent.width || 
        extent.width > surfaceCapabilities.maxImageExtent.width ||
        extent.height < surfaceCapabilities.minImageExtent.height || 
        extent.height > surfaceCapabilities.maxImageExtent.height) {
        throw std::runtime_error("Swap chain extent is outside valid bounds (window may be minimized)");
    }

    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    imageCount = numBuffers > imageCount ? numBuffers : imageCount;
    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    // --- Create swap chain ---
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    // Specify surface to be tied to
    createInfo.surface = vkSurface;

    // Add details of the swap chain
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const auto& queueFamilyIndices = instance->GetQueueFamilyIndices();
    if (queueFamilyIndices[QueueFlags::Graphics] != queueFamilyIndices[QueueFlags::Present]) {
        // Images can be used across multiple queue families without explicit ownership transfers
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        unsigned int indices[] = {
            static_cast<unsigned int>(queueFamilyIndices[QueueFlags::Graphics]),
            static_cast<unsigned int>(queueFamilyIndices[QueueFlags::Present])
        };
        createInfo.pQueueFamilyIndices = indices;
    }
    else {
        // An image is owned by one queue family at a time and ownership must be explicitly transfered between uses
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    // Specify transform on images in the swap chain (no transformation done here)
    createInfo.preTransform = surfaceCapabilities.currentTransform;

    // Specify alpha channel usage (set to be ignored here)
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Specify presentation mode
    createInfo.presentMode = presentMode;

    // Specify whether we can clip pixels that are obscured by other windows
    createInfo.clipped = VK_TRUE;

    // Reference to old swap chain in case current one becomes invalid
    createInfo.oldSwapchain = oldSwapChain;  // Use the parameter instead of VK_NULL_HANDLE

    // Create swap chain
    if (vkCreateSwapchainKHR(device->GetVkDevice(), &createInfo, nullptr, &vkSwapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain");
    }

    // --- Retrieve swap chain images ---
    vkGetSwapchainImagesKHR(device->GetVkDevice(), vkSwapChain, &imageCount, nullptr);
    vkSwapChainImages.clear();  // Clear old images first
    vkSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device->GetVkDevice(), vkSwapChain, &imageCount, vkSwapChainImages.data());

    vkSwapChainImageFormat = surfaceFormat.format;
    vkSwapChainExtent = extent;
    
    // Reset image index after recreation
    imageIndex = 0;

    // Destroy old swap chain after creating new one (if it exists)
    if (oldSwapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device->GetVkDevice(), oldSwapChain, nullptr);
    }
}

void SwapChain::Destroy() {
    vkDestroySwapchainKHR(device->GetVkDevice(), vkSwapChain, nullptr);
}

VkSwapchainKHR SwapChain::GetVkSwapChain() const {
    return vkSwapChain;
}

VkFormat SwapChain::GetVkImageFormat() const {
    return vkSwapChainImageFormat;
}

VkExtent2D SwapChain::GetVkExtent() const {
    return vkSwapChainExtent;
}

uint32_t SwapChain::GetIndex() const {
    return imageIndex;
}

uint32_t SwapChain::GetCount() const {
    return static_cast<uint32_t>(vkSwapChainImages.size());
}

VkImage SwapChain::GetVkImage(uint32_t index) const {
    return vkSwapChainImages[index];
}

VkSemaphore SwapChain::GetImageAvailableVkSemaphore() const {
    return imageAvailableSemaphore;

}

VkSemaphore SwapChain::GetRenderFinishedVkSemaphore() const {
    return renderFinishedSemaphore;
}

void SwapChain::Recreate() {
    VkSwapchainKHR oldSwapChain = vkSwapChain;  // Save the old swap chain handle
    // Don't set vkSwapChain to null - let Create() update it atomically after successful creation
    // If window is minimized, Create() will return early and we'll keep using the old swap chain
    Create(oldSwapChain);  // Create will update vkSwapChain and destroy oldSwapChain if successful
}

bool SwapChain::Acquire() {
    if (ENABLE_VALIDATION) {
        // the validation layer implementation expects the application to explicitly synchronize with the GPU
        vkQueueWaitIdle(device->GetQueue(QueueFlags::Present));
    }
    
    // Ensure swap chain is valid
    if (vkSwapChain == VK_NULL_HANDLE) {
        return false;
    }
    
    VkResult result = vkAcquireNextImageKHR(device->GetVkDevice(), vkSwapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Wait for all operations to complete before recreating
        vkDeviceWaitIdle(device->GetVkDevice());
        Recreate();
        return false;
    }
    
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image");
    }

    return true;
}

bool SwapChain::Present() {
    // Ensure swap chain is valid
    if (vkSwapChain == VK_NULL_HANDLE) {
        return false;
    }
    
    VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };

    // Submit result back to swap chain for presentation
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { vkSwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    VkResult result = vkQueuePresentKHR(device->GetQueue(QueueFlags::Present), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Wait for all operations to complete before recreating
        vkDeviceWaitIdle(device->GetVkDevice());
        Recreate();
        return false;
    }

    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image");
    }

    return true;
}

SwapChain::~SwapChain() {
    vkDestroySemaphore(device->GetVkDevice(), imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device->GetVkDevice(), renderFinishedSemaphore, nullptr);
    Destroy();
}
