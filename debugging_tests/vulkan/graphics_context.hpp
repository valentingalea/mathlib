#pragma once

#include <vector>
//#include <vulkan/vulkan.hpp>

struct queue_families_t
{
    int32_t graphics_family = -1;
    int32_t present_family = 1;

    inline bool complete(void)
    {
        return(graphics_family >= 0 && present_family >= 0);
    }
};

struct swapchain_details_t
{
    VkSurfaceCapabilitiesKHR capabilities;

    std::vector<VkSurfaceFormatKHR> available_formats;
    std::vector<VkPresentModeKHR> available_present_modes;
};

struct gpu_t
{
    VkPhysicalDevice hardware;
    VkDevice logical_device;

    VkPhysicalDeviceMemoryProperties memory_properties;
    VkPhysicalDeviceProperties properties;

    queue_families_t queue_families;
    VkQueue graphics_queue;
    VkQueue present_queue;

    swapchain_details_t swapchain_support;
    VkFormat supported_depth_format;

    void find_queue_families(VkSurfaceKHR *surface);
};

struct swapchain_t
{
    VkFormat format;
    VkPresentModeKHR present_mode;
    VkSwapchainKHR swapchain;
    VkExtent2D extent;

    uint32_t image_count;
    std::vector<VkImage> imgs;
    std::vector<VkImageView> views;
};

struct vulkan_context_t
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    gpu_t gpu;
    VkSurfaceKHR surface;
    swapchain_t swapchain;

    VkSemaphore target_image_available_semaphore;
    VkSemaphore rendering_finished_semaphore;
    VkFence cpu_wait;

    VkCommandPool command_pool_reset;
    VkCommandBuffer primary_command_buffer;

    uint32_t current_image_index;
};

struct render_pass_attachment_t
{
    VkFormat format;
    // Initial layout is always undefined
    VkImageLayout final_layout;
};

struct render_pass_attachment_reference_t
{
    uint32_t index;
    VkImageLayout layout;
};

struct render_pass_subpass_t
{
    static constexpr uint32_t MAX_COLOR_ATTACHMENTS = 7;
    render_pass_attachment_reference_t color_attachments[MAX_COLOR_ATTACHMENTS];
    uint32_t color_attachment_count {0};

    render_pass_attachment_reference_t depth_attachment;
    bool enable_depth {0};

    render_pass_attachment_reference_t input_attachments[MAX_COLOR_ATTACHMENTS];
    uint32_t input_attachment_count {0};

    template <typename ...T> void
    set_color_attachment_references(const T &...ts)
    {
        render_pass_attachment_reference_t references[] { ts... };
        for (uint32_t i = 0; i < sizeof...(ts); ++i)
        {
            color_attachments[i] = references[i];
        }
        color_attachment_count = sizeof...(ts);
    }

    void
    set_depth(const render_pass_attachment_reference_t &reference)
    {
        enable_depth = true;
        depth_attachment = reference;
    }

    template <typename ...T> void
    set_input_attachment_references(const T &...ts)
    {
        render_pass_attachment_reference_t references[] { ts... };
        for (uint32_t i = 0; i < sizeof...(ts); ++i)
        {
            input_attachments[i] = references[i];
        }
        input_attachment_count = sizeof...(ts);
    }
};

struct render_pass_dependency_t
{
    int32_t src_index;
    VkPipelineStageFlags src_stage;
    uint32_t src_access;

    int32_t dst_index;
    VkPipelineStageFlags dst_stage;
    uint32_t dst_access;
};

render_pass_dependency_t make_render_pass_dependency(int32_t src_index, VkPipelineStageFlags src_stage, uint32_t src_access,
                                                     int32_t dst_index, VkPipelineStageFlags dst_stage, uint32_t dst_access);
void initialize_render_pass(const std::vector<render_pass_attachment_t> &attachments,
                            const std::vector<render_pass_subpass_t> &subpasses,
                            const std::vector<render_pass_dependency_t> &dependencies,
                            bool clear_every_frame,
                            VkRenderPass *render_pass);

inline VkRect2D make_render_area(const VkOffset2D &offset, const VkExtent2D &extent)
{
    VkRect2D render_area {};
    render_area.offset = offset;
    render_area.extent = extent;
    return(render_area);
}

struct framebuffer_attachment_t
{
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
};

struct framebuffer_t
{
    VkFramebuffer framebuffer;
    
    std::vector<framebuffer_attachment_t> color_attachments;
    framebuffer_attachment_t depth_attachment;
    
    VkExtent2D extent;
    uint32_t layer_count;
};

template <typename ...Clears> void
begin_render_pass(VkCommandBuffer command_buffer,
                  VkRenderPass render_pass,
                  framebuffer_t &framebuffer,
                  VkSubpassContents contents,
                  const Clears &...clear_values)
{
    VkClearValue clears[sizeof...(clear_values) + 1] = {clear_values..., VkClearValue{}};

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.pNext = nullptr;
    render_pass_info.renderPass = render_pass;

    render_pass_info.framebuffer = framebuffer.framebuffer;
    render_pass_info.renderArea = make_render_area({0, 0}, framebuffer.extent);

    render_pass_info.clearValueCount = sizeof...(clear_values);
    render_pass_info.pClearValues = clears;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, contents);
}

inline void next_subpass(VkCommandBuffer command_buffer ,VkSubpassContents contents)
{
    vkCmdNextSubpass(command_buffer, contents);
}

inline void end_render_pass(VkCommandBuffer command_buffer)
{
    vkCmdEndRenderPass(command_buffer);
}

void initialize_framebuffer_attachment(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, framebuffer_attachment_t *attachment);
void initialize_framebuffer(const std::vector<framebuffer_attachment_t> &color_attachments,
                            const framebuffer_attachment_t &depth_attachment,
                            const VkExtent2D &extent,
                            VkRenderPass compatible_render_pass,
                            framebuffer_t *framebuffer);

inline VkClearValue make_clear_color_color(float r, float g, float b, float a)
{
    VkClearValue value {};
    value.color = {r, g, b, a};
    return(value);
}

inline VkClearValue make_clear_color_depth(float d, uint32_t s)
{
    VkClearValue value {};
    value.depthStencil = {d, s};
    return(value);
}

struct graphics_pipeline_t
{
    VkPipelineLayout layout;
    VkPipeline pipeline;
};

struct shader_module_info_t
{
    const char *filename;
    VkShaderStageFlagBits stage;
};

struct shader_modules_t
{
    static constexpr uint32_t MAX_SHADERS = 5;
    shader_module_info_t modules[MAX_SHADERS];
    uint32_t count;
    
    template <typename ...T>
    shader_modules_t(T ...modules_p)
        : modules{modules_p...}, count(sizeof...(modules_p))
    {
    }
};

struct shader_uniform_layouts_t
{
    static constexpr uint32_t MAX_LAYOUTS = 10;
    VkDescriptorSetLayout layouts[MAX_LAYOUTS];
    uint32_t count;
    
    template <typename ...T>
    shader_uniform_layouts_t(T ...layouts_p)
        : layouts{layouts_p...}, count(sizeof...(layouts_p))
    {
    }
};

struct shader_pk_data_t
{
    uint32_t size;
    uint32_t offset;
    VkShaderStageFlags stages;
};

struct shader_blend_states_t
{
    static constexpr uint32_t MAX_BLEND_STATES = 10;
    // For now, is just boolean
    bool blend_states[MAX_BLEND_STATES];
    uint32_t count;
    
    template <typename ...T>
    shader_blend_states_t(T ...states)
        : blend_states{states...}, count(sizeof...(states))
    {
    }
};

struct dynamic_states_t
{
    static constexpr uint32_t MAX_DYNAMIC_STATES = 10;
    VkDynamicState dynamic_states[MAX_DYNAMIC_STATES];
    uint32_t count;

    template <typename ...T>
    dynamic_states_t(T ...states)
        : dynamic_states{states...}, count(sizeof...(states))
    {
    }
};

struct model_binding_t
{
    // buffer that stores all the attributes
    VkBuffer buffer;
    uint32_t binding;
    VkVertexInputRate input_rate;

    VkVertexInputAttributeDescription *attribute_list = nullptr;
    uint32_t stride = 0;
	
    void
    begin_attributes_creation(VkVertexInputAttributeDescription *attribute_list)
    {
        this->attribute_list = attribute_list;
    }
	
    void
    push_attribute(uint32_t location, VkFormat format, uint32_t size)
    {
        VkVertexInputAttributeDescription *attribute = &attribute_list[location];
	    
        attribute->binding = binding;
        attribute->location = location;
        attribute->format = format;
        attribute->offset = stride;

        stride += size;
    }

    void
    end_attributes_creation(void)
    {
        attribute_list = nullptr;
    }
};

struct draw_indexed_data_t
{
    uint32_t index_count;
    uint32_t instance_count;
    uint32_t first_index;
    uint32_t vertex_offset;
    uint32_t first_instance;
};

inline draw_indexed_data_t
init_draw_indexed_data_default(uint32_t instance_count,
                               uint32_t index_count)
{
    draw_indexed_data_t index_data = {};
    index_data.index_count = index_count;
    index_data.instance_count = instance_count;
    index_data.first_index = 0;
    index_data.vertex_offset = 0;
    index_data.first_instance = 0;
    return(index_data);
}

struct model_index_data_t
{
    VkBuffer index_buffer;
	
    uint32_t index_count;
    uint32_t index_offset;
    VkIndexType index_type;

    draw_indexed_data_t
    init_draw_indexed_data(uint32_t first_index,
                           uint32_t offset)
    {
        draw_indexed_data_t data;
        data.index_count = index_count;
        // default the instance_count to 0
        data.instance_count = 1;
        data.first_index = 0;
        data.vertex_offset = offset;
        data.first_instance = 0;

        return(data);
    }
};

// describes the attributes and bindings of the model
struct model_t
{
    std::vector<model_binding_t> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    model_index_data_t index_data;

    std::vector<VkBuffer> raw_cache_for_rendering;
	
    std::vector<VkVertexInputBindingDescription>
    create_binding_descriptions(void)
    {
        
        std::vector<VkVertexInputBindingDescription> descriptions;
        descriptions.resize(bindings.size());
        for (uint32_t i = 0; i < bindings.size(); ++i)
        {
            descriptions[i].binding = bindings[i].binding;
            descriptions[i].stride = bindings[i].stride;
            descriptions[i].inputRate = bindings[i].input_rate;
        }
        return(descriptions);
    }

    void
    create_vertex_input_state_info(VkPipelineVertexInputStateCreateInfo *info)
    {
        std::vector<VkVertexInputBindingDescription> binding_descriptions = create_binding_descriptions();

        info->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	
        info->vertexBindingDescriptionCount = binding_descriptions.size();
        info->pVertexBindingDescriptions = binding_descriptions.data();
        info->vertexAttributeDescriptionCount = attributes.size();
        info->pVertexAttributeDescriptions = attributes.data();
    }

    void
    create_vbo_list(void)
    {
        raw_cache_for_rendering.resize(bindings.size());
	    
        for (uint32_t i = 0; i < bindings.size(); ++i)
        {
            raw_cache_for_rendering[i] = bindings[i].buffer;
        }
    }
};

void initialize_graphics_pipeline(graphics_pipeline_t *ppln,
                                  const dynamic_states_t &dynamic,
                                  const shader_modules_t &modules,
                                  bool primitive_restart, VkPrimitiveTopology topology,
                                  VkPolygonMode polygonmode, VkCullModeFlags culling,
                                  std::vector<VkDescriptorSetLayout> &layouts,
                                  const shader_pk_data_t &pk,
                                  VkExtent2D viewport,
                                  const shader_blend_states_t &blends,
                                  model_t *model,
                                  bool enable_depth,
                                  float depth_bias,
                                  VkRenderPass *compatible,
                                  uint32_t subpass);

void initialize_image_view(VkFormat format, VkImageAspectFlags aspect_flags, VkImageViewType type, uint32_t layers, VkImage *image, VkImageView *view);
void initialize_command_pool(uint32_t family_index, VkCommandPoolCreateFlags flags, VkCommandPool *command_pool);
void initialize_semaphore(VkSemaphore *semaphore);
void initialize_fence(VkFenceCreateFlags flags, VkFence *fence);
struct swapchain_information_t
{
    VkFormat swapchain_format;
    VkFormat supported_depth_format;
    VkExtent2D swapchain_extent;
    const std::vector<VkImageView> &image_views;
};
swapchain_information_t initialize_graphics_context(window_data_t &window);

struct frame_rendering_data_t
{
    uint32_t frame_index;
    VkCommandBuffer command_buffer;
};

frame_rendering_data_t begin_frame_rendering(void);
void end_frame_rendering_and_refresh(void);
