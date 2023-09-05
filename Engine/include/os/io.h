#pragma once

NS_BEGIN(os)
NS_BEGIN(io)

New_String ST_API get_exe_path();

// NOT OS SPECIFIC

New_String ST_API get_directory(str_ptr_t path); 

New_String ST_API get_exe_dir();

NS_END(io)
NS_END(os)