library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity rfid_auto_recorder is
    generic (
        DATA_WIDTH : integer := 32;
        FIFO_DEPTH : integer := 16
    );
    port (
        clk           : in  std_logic;
        reset_n       : in  std_logic;
        
        -- D? li?u UID t? MicroBlaze (N?i v?i slv_reg0)
        uid_in        : in  std_logic_vector(DATA_WIDTH-1 downto 0);
        
        -- Giao di?n ??c (N?i v?i logic slv_reg_rden c?a slv_reg1)
        rd_en         : in  std_logic;
        uid_out       : out std_logic_vector(DATA_WIDTH-1 downto 0);
        
        -- Tr?ng thái
        fifo_empty    : out std_logic;
        fifo_full     : out std_logic
    );
end rfid_auto_recorder;

architecture Behavioral of rfid_auto_recorder is
    type mem_type is array (0 to FIFO_DEPTH-1) of std_logic_vector(DATA_WIDTH-1 downto 0);
    signal fifo_mem    : mem_type := (others => (others => '0'));
    
    signal wr_ptr      : integer range 0 to FIFO_DEPTH-1 := 0;
    signal rd_ptr      : integer range 0 to FIFO_DEPTH-1 := 0;
    signal count       : integer range 0 to FIFO_DEPTH := 0;
    signal uid_prev    : std_logic_vector(DATA_WIDTH-1 downto 0) := (others => '0');

begin
    process(clk)
    begin
        if rising_edge(clk) then
            if reset_n = '0' then
                wr_ptr <= 0; rd_ptr <= 0; count <= 0;
                uid_prev <= (others => '0');
                uid_out <= (others => '0');
            else
                -- ?i?u ki?n Ghi (W) và ??c (R)
                -- W: UID m?i xu?t hi?n và FIFO ch?a ??y
                -- R: MicroBlaze yêu c?u ??c và FIFO không tr?ng
                if (uid_in /= uid_prev and uid_in /= x"00000000" and count < FIFO_DEPTH) and (rd_en = '1' and count > 0) then
                    fifo_mem(wr_ptr) <= uid_in;
                    uid_out <= fifo_mem(rd_ptr);
                    wr_ptr <= (wr_ptr + 1) mod FIFO_DEPTH;
                    rd_ptr <= (rd_ptr + 1) mod FIFO_DEPTH;
                    uid_prev <= uid_in;
                elsif (uid_in /= uid_prev and uid_in /= x"00000000" and count < FIFO_DEPTH) then
                    fifo_mem(wr_ptr) <= uid_in;
                    wr_ptr <= (wr_ptr + 1) mod FIFO_DEPTH;
                    count <= count + 1;
                    uid_prev <= uid_in;
                elsif (rd_en = '1' and count > 0) then
                    uid_out <= fifo_mem(rd_ptr);
                    rd_ptr <= (rd_ptr + 1) mod FIFO_DEPTH;
                    count <= count - 1;
                else
                    if count = 0 then uid_out <= (others => '0'); end if;
                end if;

                -- Reset b? nh? ??m c?nh khi MicroBlaze tr? slv_reg0 v? 0
                if (uid_in = x"00000000") then uid_prev <= x"00000000"; end if;
            end if;
        end if;
    end process;

    fifo_empty <= '1' when count = 0 else '0';
    fifo_full  <= '1' when count = FIFO_DEPTH else '0';
end Behavioral;