project('Lemon Applications', default_options : ['cpp_std=c++17', 'optimization=3'])

add_languages('c', 'cpp')
subproject('LemonUtils')

fileman_src = [
    'FileManager/main.cpp',
]

lsh_src = [
    'LSh/main.cpp',
]

shell_src = [
    'Shell/main.cpp',
    'Shell/shell.cpp',
]

snake_src = [
    'Snake/main.cpp',
]

terminal_src = [
    'Terminal/main.cpp',
]

textedit_src = [
    'TextEdit/exttextbox.cpp',
    'TextEdit/main.cpp',
]

sysinfo_src = [
    'SysInfo/main.cpp',
]

lemonpaint_src = [
    'LemonPaint/main.cpp',
    'LemonPaint/canvas.cpp',
    'LemonPaint/brush.cpp',
]

imgview_src = [
    'ImgView/main.cpp',
]

solitaire_src = [
    'Solitaire/main.cpp',
]

fstest_src = [
    'FSTest/main.cpp',
]

solitaire_src = [
    'Solitaire/main.cpp',
]

application_cpp_args = [
    
]

guitest_src = [
    'GUITest/main.cpp',
]

netgov_src = [
    'NetworkGovernor/main.cpp',
]

executable('fileman.lef', fileman_src, cpp_args : application_cpp_args, link_args : ['-llemon', '-lfreetype', '-lz', '-lpng'], install : true)
executable('lsh.lef', lsh_src, cpp_args : application_cpp_args, link_args : ['-llemon'], install : true)
executable('shell.lef', shell_src, cpp_args : application_cpp_args, link_args : ['-llemon', '-lfreetype', '-lz', '-lpng'], install : true)
executable('snake.lef', snake_src, cpp_args : application_cpp_args, link_args : ['-llemon', '-lfreetype'], install : true)
executable('terminal.lef', terminal_src, cpp_args : application_cpp_args, link_args : ['-llemon', '-lfreetype'], install : true)
executable('textedit.lef', textedit_src, cpp_args : application_cpp_args, link_args : ['-llemon', '-lfreetype', '-lz', '-lpng'], install : true)
executable('lemonpaint.lef', lemonpaint_src, cpp_args : application_cpp_args, link_args : ['-llemon', '-lfreetype', '-lz', '-lpng'], install : true)
executable('imgview.lef', imgview_src, cpp_args : application_cpp_args, link_args : ['-llemon', '-lfreetype', '-lz', '-lpng'], install : true)
executable('sysinfo.lef', sysinfo_src, cpp_args : application_cpp_args, link_args : ['-llemon', '-lfreetype', '-lz', '-lpng'], install : true)
executable('guitest.lef', guitest_src, cpp_args : application_cpp_args, link_args : ['-llemon', '-lfreetype', '-lz', '-lpng'], install : true)
