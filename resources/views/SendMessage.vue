<template>
  <form class="container-send-message" @submit.prevent="onSubmit">
    <TextInput
        type="text"
        v-model="content"
        name="content"
        :required="true"
        element="textarea"
      />
    <div class="row">
      <TextInput
        type="text"
        v-model="recipient"
        name="recipient"
        label="Recipient"
        :required="true"
        :maxLength="15"
      />
      <Button value="Send" type="submit"/>
    </div>
  </form>
</template>

<script>
import TextInput from "../components/TextInput.vue";
import Button from "../components/Button.vue";
export default {
  name: "CreateMessage",
  components: { TextInput, Button },
  data() {
    return {
      content: '',
      recipient: ''
    }
  },
  methods: {
    onSubmit () {
      this.$store.commit('sendMessage', {
        sender: "+37067145425",
        recipient: this.recipient,
        content: this.content
      })
    }
  }
};
</script>

<style lang="scss" scoped>
.container-send-message {
  display: flex;
  flex-direction: column;
  width: 100%;
  height: calc(69vh - 80px);
  .row {
    display: flex;
    flex-direction: row;
  }
}
@media only screen and (max-width: 980px) {
  .container-send-message {
    height: calc(85vh - 80px);
  }
}
@media only screen and (max-width: 780px) {
  .container-send-message {
    height: calc(85vh - 200px);
  }
}
</style>
