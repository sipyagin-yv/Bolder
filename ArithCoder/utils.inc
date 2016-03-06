struc           rb      length
{ .             rb      length
  .size         =       $-.
}

struc           db      [data]
{            common
  .             db      data
  .size         =       $-.
}

struct          pushad_data
r_edi           rd      1
r_esi           rd      1
r_ebp           rd      1
r_esp           rd      1
r_ebx           rd      1
r_edx           rd      1
r_ecx           rd      1
r_eax           rd      1
ends