indir = ARGV[0]
outdir = ARGV[1]

Dir.foreach(indir) do |f|
  next unless f =~ /tif$/

  infile = "#{indir}/#{f}"
  outfile = "#{outdir}/#{f.gsub('.tif', '')}"
  header = "#{outdir}/#{f.gsub('.tif', '.hdr')}"

  `./geotiff_to_gridfloat #{infile} #{outfile} #{header}`
  #exit
end
