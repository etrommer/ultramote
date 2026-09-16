# Ultramote PCB assets, built with the KiCad command line interface.
#
#   make                 build everything
#   make docs            schematic PDF, board PDF, BoM CSV
#   make ibom            interactive HTML BoM
#   make fab             JLCPCB gerbers, drill files, CPL, BoM and upload archive
#   make step            STEP model
#   make render          3D renders
#   make clean           delete the output directory
#   make OUT=<dir>       write the assets elsewhere (default: generated)
#
# Needs kicad-cli (KiCad 10) and bsdtar. The interactive BoM uses the
# InteractiveHtmlBom CLI from PATH, or the one that ships with the KiCad plugin
# installation; point IBOM= at a different one if needed.
#
# CI builds into website/static/pcb_assets, which it publishes with the site
# without committing it (see .github/workflows/website.yml).

KICAD_CLI ?= kicad-cli
TAR       ?= bsdtar

# The interactive BoM has no kicad-cli equivalent. Use an InteractiveHtmlBom CLI
# from PATH, else the one bundled with the KiCad plugin whose name sorts last
# (i.e. the newest KiCad version), else leave the bare name for the error.
IBOM_PLUGIN := $(lastword $(sort $(wildcard \
	$(HOME)/.local/share/kicad/*/3rdparty/plugins/*/generate_interactive_bom.py \
	$(HOME)/.local/share/kicad/scripting/plugins/*/generate_interactive_bom.py)))
IBOM      ?= $(or $(shell command -v generate_interactive_bom 2>/dev/null),$(IBOM_PLUGIN),generate_interactive_bom)

PCB   := ultramote
SCH   := $(PCB).kicad_sch
BOARD := $(PCB).kicad_pcb
SHEETS := $(wildcard *.kicad_sch)

OUT    ?= generated
DOCS   := $(OUT)/docs
BOMDIR := $(DOCS)/bom
IMGDIR := $(DOCS)/img
FABDIR := $(OUT)/gerbers/jlcpcb

# Board PDF: one page per plotted layer.
PCB_LAYERS := F.Cu,B.Cu,F.Silkscreen,B.Silkscreen,Edge.Cuts
FAB_LAYERS := F.Cu,B.Cu,F.Silkscreen,B.Silkscreen,F.Mask,B.Mask,Edge.Cuts

# Grouping fields shared by the BoM exports.
BOM_GROUP = Value,Footprint,Datasheet,LCSC Part Number,Manufacturer Part Number

# Orthogonal 3D view, tilted the same amount in X and Z, opaque background,
# zoomed out far enough to keep the edge-mounted connectors in frame.
RENDER_ARGS = --quality high --background opaque --width 1280 --height 720 --rotate 4,0,4 --zoom 0.75

# KiCad's BoM exporter only knows the symbol fields, so the schematic's
# library-derived Description/Part columns are not reproduced here.
BOM_FIELDS := ITEM_NUMBER,Reference,Value,Footprint,QUANTITY,Datasheet,LCSC Part Number,Manufacturer Part Number
BOM_LABELS := Row,References,Value,Footprint,Quantity Per PCB,Datasheet,LCSC Part Number,Manufacturer Part Number

.PHONY: all docs ibom fab step render clean

# Never leave a half-written export behind that make would treat as current.
.DELETE_ON_ERROR:

all: docs ibom fab step render

docs: $(DOCS)/$(PCB)-schematic.pdf $(DOCS)/$(PCB)-pcb-layers.pdf $(BOMDIR)/bom_$(PCB).csv
ibom: $(BOMDIR)/$(PCB)-ibom.html
fab: $(OUT)/$(PCB)-gerber-jlcpcb.zip
step: $(OUT)/$(PCB).step
render: $(IMGDIR)/$(PCB)-top.png $(IMGDIR)/$(PCB)-bottom.png

$(OUT) $(DOCS) $(BOMDIR) $(IMGDIR) $(FABDIR):
	mkdir -p $@

$(DOCS)/$(PCB)-schematic.pdf: $(SHEETS) | $(DOCS)
	$(KICAD_CLI) sch export pdf --output $@ $(SCH)

$(DOCS)/$(PCB)-pcb-layers.pdf: $(BOARD) | $(DOCS)
	$(KICAD_CLI) pcb export pdf --mode-multipage --scale 0 --check-zones \
		--layers $(PCB_LAYERS) --output $@ $(BOARD)

$(BOMDIR)/bom_$(PCB).csv: $(SHEETS) | $(BOMDIR)
	$(KICAD_CLI) sch export bom --output $@ \
		--fields '$(BOM_FIELDS)' --labels '$(BOM_LABELS)' \
		--group-by '$(BOM_GROUP)' \
		--ref-delimiter ' ' --ref-range-delimiter '' $(SCH)

$(BOMDIR)/$(PCB)-ibom.html: $(BOARD) | $(BOMDIR)
	$(IBOM) --no-browser --dest-dir $(BOMDIR) --name-format $(PCB)-ibom \
		--dark-mode --highlight-pin1 all --include-nets --include-tracks \
		--layer-view FB --bom-view left-right --checkboxes 'Sourced,Placed' \
		--normalize-field-case \
		--sort-order 'C,R,L,D,U,Y,X,F,SW,A,~,HS,CNN,J,P,NT,MH' $(BOARD)

# The drill files are the anchor for the whole gerber set: both exports land
# in FABDIR, so replotting either of them rebuilds this group.
$(FABDIR)/$(PCB)-PTH.drl: $(BOARD) | $(FABDIR)
	$(KICAD_CLI) pcb export gerbers --output $(FABDIR)/ --layers $(FAB_LAYERS) \
		--no-protel-ext --no-x2 --no-netlist --disable-aperture-macros \
		--exclude-value --subtract-soldermask --check-zones $(BOARD)
	$(KICAD_CLI) pcb export drill --output $(FABDIR)/ --format excellon \
		--excellon-separate-th -u in $(BOARD)

# Raw kicad-cli pick and place output: KiCad's column names and rotation
# convention, with no JLCPCB rotation corrections.
$(FABDIR)/$(PCB)-top_pos_cpl.csv: $(BOARD) | $(FABDIR)
	$(KICAD_CLI) pcb export pos --side front --format csv --units mm \
		--smd-only --output $@ $(BOARD)

$(FABDIR)/$(PCB)-bottom_pos_cpl.csv: $(BOARD) | $(FABDIR)
	$(KICAD_CLI) pcb export pos --side back --format csv --units mm \
		--smd-only --output $@ $(BOARD)

$(FABDIR)/$(PCB)-jlcpcb-bom.csv: $(SHEETS) | $(FABDIR)
	$(KICAD_CLI) sch export bom --output $@ \
		--fields 'Value,Reference,Footprint,LCSC Part Number' \
		--labels 'Comment,Designator,Footprint,LCSC Part #' \
		--group-by 'Value,Footprint,LCSC Part Number' \
		--ref-delimiter ',' --ref-range-delimiter '' $(SCH)

$(OUT)/$(PCB)-gerber-jlcpcb.zip: $(FABDIR)/$(PCB)-PTH.drl \
		$(FABDIR)/$(PCB)-top_pos_cpl.csv $(FABDIR)/$(PCB)-bottom_pos_cpl.csv \
		$(FABDIR)/$(PCB)-jlcpcb-bom.csv | $(OUT)
	cd $(FABDIR) && $(TAR) -a -cf $(abspath $@) *

# cad_models/ is a plain prerequisite (not its contents, which include a name
# with a space) so that adding or updating a model rebuilds the export.
$(OUT)/$(PCB).step: $(BOARD) cad_models | $(OUT)
	$(KICAD_CLI) pcb export step --force --subst-models --output $@ $(BOARD)

$(IMGDIR)/$(PCB)-top.png: $(BOARD) | $(IMGDIR)
	$(KICAD_CLI) pcb render --side top $(RENDER_ARGS) --output $@ $(BOARD)

$(IMGDIR)/$(PCB)-bottom.png: $(BOARD) | $(IMGDIR)
	$(KICAD_CLI) pcb render --side bottom $(RENDER_ARGS) --output $@ $(BOARD)

clean:
	rm -rf $(OUT)
