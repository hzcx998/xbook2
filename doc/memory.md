# 内存管理
分页内存映射机制：
* page_link_addr：va虚拟地址和pa物理地址进行连接，虚拟地址必须是没有连接过的地址才行
* page_link_addr_unsafe：连接地址，如果虚拟地址里面已经存放了物理地址，那么就需要先释放对应的物理地址，然后再进行地址连接
* page_unlink_addr: 修清除改虚拟地址对应的物理页连接，并释放对应的物理页
* page_map_addr: 映射一片区域的虚拟地址，物理地址是根据需要分配（安全连接）
* page_map_addr_fixed: 映射一片区域的虚拟地址，物理地址使用指定的地址（安全连接）
* page_map_addr_safe: 映射一片内存区域，如果虚拟地址对应的物理地址不存在才为其分配物理地址，已经存在就默认使用原来的物理页。
* page_unmap_addr: 取消一片内存区域的映射，会释放虚拟地址里面的物理页
* page_unmap_addr_safe: 取消一片内存区域的映射，会释放虚拟地址里面的物理页。如果存在物理页才释放物理页。如果是固定的区域，那么就不会释放物理页。

# 进程执行时，内存状况
load加载代码段和数据段：mem_space_mmap -> SHARE ? page_map_addr_fixed(单页) : page_map_addr_safe (单页)
heap堆：do_handle_no_page -> page_map_addr (单页)
stack： 初始化时：mem_space_mmap -> page_map_addr_safe (单页)
        扩展时：do_handle_no_page -> page_map_addr (单页)
        