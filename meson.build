project('libplacebo', ['c', 'cpp'],
  license: 'LGPL2.1+',
  default_options: ['c_std=c11', 'cpp_std=c++11', 'warning_level=2'],
  meson_version: '>=0.54',
  version: '@0@.@1@.@2@'.format(
    # Major version
    4,
    # API version
    157 + {
      # Incremental log of API changes (since v4.157.0)
      '158': 'add support for H.274 film grain',
      '159': 'remove fields deprecated for libplacebo v3',
      '160': 'add preprocessor macros for default params',
      '161': 'make H.274 film grain values indirect',
      '162': 'support rotation in pl_renderer',
      '163': 'add pl_frame_copy_stream_props',
      '164': 'support blending against tiles',
      '165': 'add pl_fmt.signature',
      '166': 'add pl_index_format',
      '167': 'expose pl_dispatch_reset_frame',
      '168': 'refactor pl_tex_transfer.stride_w/h into row/depth_pitch',
      '169': 'refactor pl_pass_params.target_dummy into target_format',
      '170': 'allow pl_queue_update on NULL',
      '171': 'make vulkan 1.2 the minimum version',
      '172': 'replace VkSemaphore by pl_vulkan_sem in pl_vulkan_hold/release',
      '173': 'remove VkAccessFlags from pl_vulkan_hold/release',
      '174': 'deprecate pl_vulkan_params.disable_events',
      '175': 'require timeline semaphores for all vulkan devices',
      '176': 'revert vulkan 1.2 requirement',
      '177': 'add debug_tag to pl_tex/buf_params',
      '178': 'add pl_gpu_limits.align_vertex_stride',
      '179': 'add pl_render_params.skip_caching_single_frame',
      '180': 'add pl_gpu_limits.max_variable_comps',
      '181': 'add pl_shader_set_alpha, change alpha handling of pl_shader_decode_color',
      '182': 'add pl_vulkan_get, pl_opengl_get, pl_d3d11_get',
      '183': 'relax pl_shared_mem.size > 0 requirement',
      '184': 'add pl_map_avframe/pl_unmap_avframe, deprecate pl_upload_avframe',
      '185': 'add PL_COLOR_SYSTEM_DOLBYVISION and reshaping',
      '186': 'add pl_d3d11_swapchain_params.flags',
      '187': 'add <libplacebo/tone_mapping.h>',
      '188': 'refactor pl_color_map_params tone mapping settings',
      '189': 'refactor pl_color_space, merging it with pl_hdr_metadata',
      '190': 'add pl_color_map_params.gamut_mode, replacing gamut_clipping/warning',
      '191': 'add pl_map_dovi_metadata',
      '192': 'add pl_map_avframe_ex',
    }.keys().length(),
    # Fix version
    0)
)

# Version number
version = meson.project_version()
version_pretty = 'v' + version
version_split = version.split('.')

majorver = version_split[0]
apiver = version_split[1]
fixver = version_split[2]

proj_name = meson.project_name()

# Project build options
build_opts = [
  # Warnings
  '-Wundef', '-Wshadow', '-Wparentheses', '-Wpointer-arith',
]

link_args = []

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

c_opts = [
  '-D_ISOC99_SOURCE', '-D_ISOC11_SOURCE', '-D_GNU_SOURCE', '-D_XOPEN_SOURCE=700',
  '-U__STRICT_ANSI__', '-fvisibility=hidden', '-Wmissing-prototypes',

  # Warnings to ignore
  '-Wno-pointer-sign', '-Wno-sign-compare', '-Wno-unused-parameter',
  '-Wno-missing-field-initializers', '-Wno-type-limits',

  # Warnings to treat as errors
  '-Werror=implicit-function-declaration',
]

# glslang needs c++11
cpp_opts = [
  '-fvisibility=hidden',
]

if cc.has_argument('-Wincompatible-pointer-types')
  c_opts += ['-Werror=incompatible-pointer-types']
endif

# clang's version of -Wmissing-braces rejects the common {0} initializers
if cc.get_id() == 'clang'
  c_opts += ['-Wno-missing-braces']
endif

# don't leak library symbols if possible
vflag = '-Wl,--exclude-libs=ALL'
if cc.has_link_argument(vflag)
  link_args += [vflag]
endif

# OS specific build options
if host_machine.system() == 'windows'
    build_opts += ['-D_WIN32_WINNT=0x0601']
endif

add_project_arguments(build_opts + c_opts, language: 'c')
add_project_arguments(build_opts + cpp_opts, language: 'cpp')
add_project_link_arguments(link_args, language: 'c')

subdir('src')

if get_option('demos')
  subdir('demos')
endif
