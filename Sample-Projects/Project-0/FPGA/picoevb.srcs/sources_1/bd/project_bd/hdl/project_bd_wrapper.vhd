--Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
----------------------------------------------------------------------------------
--Tool Version: Vivado v.2019.2 (win64) Build 2700185 Thu Oct 24 18:46:05 MDT 2019
--Date        : Wed Nov 20 14:29:02 2019
--Host        : DESKTOP-FKBOMH7 running 64-bit major release  (build 9200)
--Command     : generate_target project_bd_wrapper.bd
--Design      : project_bd_wrapper
--Purpose     : IP block netlist
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity project_bd_wrapper is
  port (
    SPI_0_io0_io : inout STD_LOGIC;
    SPI_0_io1_io : inout STD_LOGIC;
    SPI_0_io2_io : inout STD_LOGIC;
    SPI_0_io3_io : inout STD_LOGIC;
    SPI_0_ss_io : inout STD_LOGIC_VECTOR ( 0 to 0 );
    auxio_tri_io : inout STD_LOGIC_VECTOR ( 3 downto 0 );
    clkreq_l : out STD_LOGIC_VECTOR ( 0 to 0 );
    di_edge : in STD_LOGIC_VECTOR ( 1 downto 0 );
    do_edge : out STD_LOGIC_VECTOR ( 1 downto 0 );
    pcie_mgt_rxn : in STD_LOGIC_VECTOR ( 0 to 0 );
    pcie_mgt_rxp : in STD_LOGIC_VECTOR ( 0 to 0 );
    pcie_mgt_txn : out STD_LOGIC_VECTOR ( 0 to 0 );
    pcie_mgt_txp : out STD_LOGIC_VECTOR ( 0 to 0 );
    status_leds : out STD_LOGIC_VECTOR ( 2 downto 0 );
    sys_clk_n : in STD_LOGIC_VECTOR ( 0 to 0 );
    sys_clk_p : in STD_LOGIC_VECTOR ( 0 to 0 );
    sys_rst_n : in STD_LOGIC
  );
end project_bd_wrapper;

architecture STRUCTURE of project_bd_wrapper is
  component project_bd is
  port (
    di_edge : in STD_LOGIC_VECTOR ( 1 downto 0 );
    sys_rst_n : in STD_LOGIC;
    status_leds : out STD_LOGIC_VECTOR ( 2 downto 0 );
    clkreq_l : out STD_LOGIC_VECTOR ( 0 to 0 );
    do_edge : out STD_LOGIC_VECTOR ( 1 downto 0 );
    auxio_tri_i : in STD_LOGIC_VECTOR ( 3 downto 0 );
    auxio_tri_o : out STD_LOGIC_VECTOR ( 3 downto 0 );
    auxio_tri_t : out STD_LOGIC_VECTOR ( 3 downto 0 );
    sys_clk_p : in STD_LOGIC_VECTOR ( 0 to 0 );
    sys_clk_n : in STD_LOGIC_VECTOR ( 0 to 0 );
    pcie_mgt_rxn : in STD_LOGIC_VECTOR ( 0 to 0 );
    pcie_mgt_rxp : in STD_LOGIC_VECTOR ( 0 to 0 );
    pcie_mgt_txn : out STD_LOGIC_VECTOR ( 0 to 0 );
    pcie_mgt_txp : out STD_LOGIC_VECTOR ( 0 to 0 );
    SPI_0_io0_i : in STD_LOGIC;
    SPI_0_io0_o : out STD_LOGIC;
    SPI_0_io0_t : out STD_LOGIC;
    SPI_0_io1_i : in STD_LOGIC;
    SPI_0_io1_o : out STD_LOGIC;
    SPI_0_io1_t : out STD_LOGIC;
    SPI_0_io2_i : in STD_LOGIC;
    SPI_0_io2_o : out STD_LOGIC;
    SPI_0_io2_t : out STD_LOGIC;
    SPI_0_io3_i : in STD_LOGIC;
    SPI_0_io3_o : out STD_LOGIC;
    SPI_0_io3_t : out STD_LOGIC;
    SPI_0_ss_i : in STD_LOGIC_VECTOR ( 0 to 0 );
    SPI_0_ss_o : out STD_LOGIC_VECTOR ( 0 to 0 );
    SPI_0_ss_t : out STD_LOGIC
  );
  end component project_bd;
  component IOBUF is
  port (
    I : in STD_LOGIC;
    O : out STD_LOGIC;
    T : in STD_LOGIC;
    IO : inout STD_LOGIC
  );
  end component IOBUF;
  signal SPI_0_io0_i : STD_LOGIC;
  signal SPI_0_io0_o : STD_LOGIC;
  signal SPI_0_io0_t : STD_LOGIC;
  signal SPI_0_io1_i : STD_LOGIC;
  signal SPI_0_io1_o : STD_LOGIC;
  signal SPI_0_io1_t : STD_LOGIC;
  signal SPI_0_io2_i : STD_LOGIC;
  signal SPI_0_io2_o : STD_LOGIC;
  signal SPI_0_io2_t : STD_LOGIC;
  signal SPI_0_io3_i : STD_LOGIC;
  signal SPI_0_io3_o : STD_LOGIC;
  signal SPI_0_io3_t : STD_LOGIC;
  signal SPI_0_ss_i_0 : STD_LOGIC_VECTOR ( 0 to 0 );
  signal SPI_0_ss_io_0 : STD_LOGIC_VECTOR ( 0 to 0 );
  signal SPI_0_ss_o_0 : STD_LOGIC_VECTOR ( 0 to 0 );
  signal SPI_0_ss_t : STD_LOGIC;
  signal auxio_tri_i_0 : STD_LOGIC_VECTOR ( 0 to 0 );
  signal auxio_tri_i_1 : STD_LOGIC_VECTOR ( 1 to 1 );
  signal auxio_tri_i_2 : STD_LOGIC_VECTOR ( 2 to 2 );
  signal auxio_tri_i_3 : STD_LOGIC_VECTOR ( 3 to 3 );
  signal auxio_tri_io_0 : STD_LOGIC_VECTOR ( 0 to 0 );
  signal auxio_tri_io_1 : STD_LOGIC_VECTOR ( 1 to 1 );
  signal auxio_tri_io_2 : STD_LOGIC_VECTOR ( 2 to 2 );
  signal auxio_tri_io_3 : STD_LOGIC_VECTOR ( 3 to 3 );
  signal auxio_tri_o_0 : STD_LOGIC_VECTOR ( 0 to 0 );
  signal auxio_tri_o_1 : STD_LOGIC_VECTOR ( 1 to 1 );
  signal auxio_tri_o_2 : STD_LOGIC_VECTOR ( 2 to 2 );
  signal auxio_tri_o_3 : STD_LOGIC_VECTOR ( 3 to 3 );
  signal auxio_tri_t_0 : STD_LOGIC_VECTOR ( 0 to 0 );
  signal auxio_tri_t_1 : STD_LOGIC_VECTOR ( 1 to 1 );
  signal auxio_tri_t_2 : STD_LOGIC_VECTOR ( 2 to 2 );
  signal auxio_tri_t_3 : STD_LOGIC_VECTOR ( 3 to 3 );
begin
SPI_0_io0_iobuf: component IOBUF
     port map (
      I => SPI_0_io0_o,
      IO => SPI_0_io0_io,
      O => SPI_0_io0_i,
      T => SPI_0_io0_t
    );
SPI_0_io1_iobuf: component IOBUF
     port map (
      I => SPI_0_io1_o,
      IO => SPI_0_io1_io,
      O => SPI_0_io1_i,
      T => SPI_0_io1_t
    );
SPI_0_io2_iobuf: component IOBUF
     port map (
      I => SPI_0_io2_o,
      IO => SPI_0_io2_io,
      O => SPI_0_io2_i,
      T => SPI_0_io2_t
    );
SPI_0_io3_iobuf: component IOBUF
     port map (
      I => SPI_0_io3_o,
      IO => SPI_0_io3_io,
      O => SPI_0_io3_i,
      T => SPI_0_io3_t
    );
SPI_0_ss_iobuf_0: component IOBUF
     port map (
      I => SPI_0_ss_o_0(0),
      IO => SPI_0_ss_io(0),
      O => SPI_0_ss_i_0(0),
      T => SPI_0_ss_t
    );
auxio_tri_iobuf_0: component IOBUF
     port map (
      I => auxio_tri_o_0(0),
      IO => auxio_tri_io(0),
      O => auxio_tri_i_0(0),
      T => auxio_tri_t_0(0)
    );
auxio_tri_iobuf_1: component IOBUF
     port map (
      I => auxio_tri_o_1(1),
      IO => auxio_tri_io(1),
      O => auxio_tri_i_1(1),
      T => auxio_tri_t_1(1)
    );
auxio_tri_iobuf_2: component IOBUF
     port map (
      I => auxio_tri_o_2(2),
      IO => auxio_tri_io(2),
      O => auxio_tri_i_2(2),
      T => auxio_tri_t_2(2)
    );
auxio_tri_iobuf_3: component IOBUF
     port map (
      I => auxio_tri_o_3(3),
      IO => auxio_tri_io(3),
      O => auxio_tri_i_3(3),
      T => auxio_tri_t_3(3)
    );
project_bd_i: component project_bd
     port map (
      SPI_0_io0_i => SPI_0_io0_i,
      SPI_0_io0_o => SPI_0_io0_o,
      SPI_0_io0_t => SPI_0_io0_t,
      SPI_0_io1_i => SPI_0_io1_i,
      SPI_0_io1_o => SPI_0_io1_o,
      SPI_0_io1_t => SPI_0_io1_t,
      SPI_0_io2_i => SPI_0_io2_i,
      SPI_0_io2_o => SPI_0_io2_o,
      SPI_0_io2_t => SPI_0_io2_t,
      SPI_0_io3_i => SPI_0_io3_i,
      SPI_0_io3_o => SPI_0_io3_o,
      SPI_0_io3_t => SPI_0_io3_t,
      SPI_0_ss_i(0) => SPI_0_ss_i_0(0),
      SPI_0_ss_o(0) => SPI_0_ss_o_0(0),
      SPI_0_ss_t => SPI_0_ss_t,
      auxio_tri_i(3) => auxio_tri_i_3(3),
      auxio_tri_i(2) => auxio_tri_i_2(2),
      auxio_tri_i(1) => auxio_tri_i_1(1),
      auxio_tri_i(0) => auxio_tri_i_0(0),
      auxio_tri_o(3) => auxio_tri_o_3(3),
      auxio_tri_o(2) => auxio_tri_o_2(2),
      auxio_tri_o(1) => auxio_tri_o_1(1),
      auxio_tri_o(0) => auxio_tri_o_0(0),
      auxio_tri_t(3) => auxio_tri_t_3(3),
      auxio_tri_t(2) => auxio_tri_t_2(2),
      auxio_tri_t(1) => auxio_tri_t_1(1),
      auxio_tri_t(0) => auxio_tri_t_0(0),
      clkreq_l(0) => clkreq_l(0),
      di_edge(1 downto 0) => di_edge(1 downto 0),
      do_edge(1 downto 0) => do_edge(1 downto 0),
      pcie_mgt_rxn(0) => pcie_mgt_rxn(0),
      pcie_mgt_rxp(0) => pcie_mgt_rxp(0),
      pcie_mgt_txn(0) => pcie_mgt_txn(0),
      pcie_mgt_txp(0) => pcie_mgt_txp(0),
      status_leds(2 downto 0) => status_leds(2 downto 0),
      sys_clk_n(0) => sys_clk_n(0),
      sys_clk_p(0) => sys_clk_p(0),
      sys_rst_n => sys_rst_n
    );
end STRUCTURE;
