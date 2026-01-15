library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity rfid_storage is
    Port (
        clk      : in  STD_LOGIC;                        
        reset    : in  STD_LOGIC;                         
        uid_in   : in  STD_LOGIC_VECTOR(31 downto 0);     
        strobe   : in  STD_LOGIC;                         
        valid_out: out STD_LOGIC                         
    );
end rfid_storage;

architecture Behavioral of rfid_storage is
   
    type rfid_array is array (0 to 2) of STD_LOGIC_VECTOR(31 downto 0);
    
   
    constant AUTHORIZED_CARDS : rfid_array := (
        x"5A733D02", -- Th? s? 1: 5A 73 3D 0212311
        x"6F0A201F", -- Th? s? 2: 6F 0A 20 1F
        x"FF9B571E"  -- Th? s? 3: FF 9B 57 1E
    );

begin
    process(clk, reset)
    begin
        if reset = '0' then
            valid_out <= '0';
        elsif rising_edge(clk) then
            if strobe = '1' then
                -- Ki?m tra xem th? quét vào có n?m trong danh sách không
                if (uid_in = AUTHORIZED_CARDS(0) or 
                    uid_in = AUTHORIZED_CARDS(1) or 
                    uid_in = AUTHORIZED_CARDS(2)) then
                    valid_out <= '1'; -- Th? h?p l?
                else
                    valid_out <= '0'; -- Th? l?
                end if;
            end if;
        end if;
    end process;
end Behavioral;
