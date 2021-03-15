<template>

</template>
<script>
export default {
  name: "StoreInit",
  beforeCreate () {
      this.$store.dispatch('initStore')
  }
};
</script>