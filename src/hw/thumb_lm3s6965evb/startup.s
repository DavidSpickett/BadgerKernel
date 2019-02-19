.4byte stack_top
.4byte _Reset
# skipping vectors we aren't using...

.global _Reset
.thumb_func
_Reset:
 B entry
