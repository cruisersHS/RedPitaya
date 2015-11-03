////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya arbitrary signal generator testbench.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module asg_top_tb #(
  // time period
  realtime  TP = 8.0ns,  // 125MHz
  // data parameters
  int unsigned DWO = 14,  // RAM data width
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16,  // counter width fraction  (fixed point fraction)
  // trigger parameters
  int unsigned TWA =  4,          // external trigger array  width
  int unsigned TWS = $clog2(TWA)  // external trigger select width
);

////////////////////////////////////////////////////////////////////////////////
// DAC signal generation
////////////////////////////////////////////////////////////////////////////////

// syste signals
logic                  clk ;
logic                  rstn;
// stream
logic signed [DWO-1:0] str_dat;  // data
logic                  str_vld;  // valid
logic                  str_rdy;  // ready
// trigger
logic        [TWA-1:0] trg_ext;
logic                  trg_swo;
logic                  trg_out;

// DAC clock
initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

// DAC reset
initial begin
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  rstn = 1'b1;
end

// ADC cycle counter
int unsigned dac_cyc=0;
always_ff @ (posedge clk)
dac_cyc <= dac_cyc+1;

always begin
  trg_ext <= 1'b0 ;
  repeat(100000) @(posedge clk);
  trg_ext <= 1'b1 ;
  repeat(1200) @(posedge clk);
  trg_ext <= 1'b0 ;
end

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

logic [ 32-1: 0] sys_addr ;
logic [ 32-1: 0] sys_wdata;
logic [  4-1: 0] sys_sel  ;
logic            sys_wen  ;
logic            sys_ren  ;
logic [ 32-1: 0] sys_rdata;
logic            sys_err  ;
logic            sys_ack  ;

logic        [ 32-1: 0] rdata;
logic signed [ 32-1: 0] rdata_blk [];

////////////////////////////////////////////////////////////////////////////////
// signal generation
////////////////////////////////////////////////////////////////////////////////

localparam ADR_BUF = 1 << (CWM+2);

//int buf_len = 2**CWM;
int buf_len = 8;
real freq  = 1_000_000; // 1MHz
real phase = 0; // DEG

initial begin
  repeat(10) @(posedge clk);
  // write table and table size
  for (int i=0; i<buf_len; i++) begin
    bus.write(ADR_BUF + (i*4), i);  // write table
  end
  bus.write(32'h08, buf_len);  // table size
  // configure frequency and phase
  bus.write(32'h0C, (buf_len * (phase/360.0)) * 2**CWF);  // offset
  bus.write(32'h10, (buf_len * (freq*TP)    ) * 2**CWF);  // step
  // configure burst mode
  bus.write(32'h18, 0);  // number of cycles
  bus.write(32'h1C, 5);  // number of repetitions
  bus.write(32'h20, 10);  // number of delay periods between repetitions
  // configure amplitude and DC offset
  bus.write(32'h24,{2'h0, 14'd0, 2'h0, 14'h2000}    );  // DC offset, amplitude
  // start
  bus.write(32'h00,{8'h0}  ); // write configuration

  repeat(200) @(posedge clk);

  // CH1 table data readback
  rdata_blk = new [80];
  for (int i=0; i<buf_len; i++) begin
    bus.read(ADR_BUF + (i*4), rdata_blk [i]);  // read table
  end

  repeat(2000) @(posedge clk);

  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

sys_bus_model bus (
  // system signals
  .clk          (clk      ),
  .rstn         (rstn     ),
  // bus protocol signals
  .sys_addr     (sys_addr ),
  .sys_wdata    (sys_wdata),
  .sys_sel      (sys_sel  ),
  .sys_wen      (sys_wen  ),
  .sys_ren      (sys_ren  ),
  .sys_rdata    (sys_rdata),
  .sys_err      (sys_err  ),
  .sys_ack      (sys_ack  ) 
);

asg_top asg_top (
  // system signals
  .clk       (clk ),
  .rstn      (rstn),
  // stream output
  .sto_dat   (str_dat),
  .sto_vld   (str_vld),
  .sto_rdy   (str_rdy),
  // triggers
  .trg_ext   (trg_ext),
  .trg_swo   (trg_swo),
  .trg_out   (trg_out),
  // System bus
  .sys_addr  (sys_addr ),
  .sys_wdata (sys_wdata),
  .sys_sel   (sys_sel  ),
  .sys_wen   (sys_wen  ),
  .sys_ren   (sys_ren  ),
  .sys_rdata (sys_rdata),
  .sys_err   (sys_err  ),
  .sys_ack   (sys_ack  )
);

// stream drain
assign str_rdy = 1'b1;

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("asg_top_tb.vcd");
  $dumpvars(0, asg_top_tb);
end

endmodule: asg_top_tb
